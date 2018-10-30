//
//  main.c
//  grab_locker
//
//  Created by Joshua Hill on 1/7/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <syslog.h>
#include <CoreFoundation/CoreFoundation.h>
#include "IOKit.h"
#include "IOAESAccelerator.h"
#include "AppleKeyStore.h"
#include "util.h"
#include "device_info.h"

void saveKeybagInfos(CFDataRef kbkeys, KeyBag* kb, uint8_t* key835, char* passcode, uint8_t* passcodeKey, CFMutableDictionaryRef classKeys)
{
    CFMutableDictionaryRef out = device_info(-1, NULL);
    
    CFStringRef uuid = CreateHexaCFString(kb->uuid, 16);
    
    CFDictionaryAddValue(out, CFSTR("uuid"), uuid);
    CFDictionaryAddValue(out, CFSTR("KeyBagKeys"), kbkeys);
    
    addHexaString(out, CFSTR("salt"), kb->salt, 20);
    
    if (passcode != NULL)
    {
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
    
    CFStringRef resultsFileName = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@.plist"), CFDictionaryGetValue(out, CFSTR("dataVolumeUUID")));
    
    CFStringRef printString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("Writing results to %@.plist\n"), CFDictionaryGetValue(out, CFSTR("dataVolumeUUID")));
    
    CFShow(printString);
    CFRelease(printString);
    
    saveResults(resultsFileName, out);
    
    CFRelease(resultsFileName);
    CFRelease(uuid);
    CFRelease(out);
    
}

int grab_system(int argc, char* argv[]) {
    char* passcode = NULL;
    uint8_t* key835 = NULL;
    uint8_t* passcodeKey = malloc(32);
    
    if(argc == 3) {
        passcode = argv[2];
        key835 = str2hex(argv[1]);
    } else {
        fprintf(stderr, "Usage: grab_system $KEY835 $PASSCODE\n");
        return -1;
    }
    
    syslog(LOG_ERR, "Pwning SystemKey!!!");
    
    syslog(LOG_ERR, "Asking AppleKeyStore to load the system keybag");
    CFDictionaryRef kbdict = AppleKeyStore_loadKeyBag("/private/var/keybags","systembag");
    if (kbdict == NULL) {
            syslog(LOG_ERR, "FAILed to load keybag\n");
            return -1;
    }
    syslog(LOG_ERR, "Successfully loaded keybag\n");
    
    syslog(LOG_ERR, "Getting keys from system keybag");
    CFDataRef kbkeys = CFDictionaryGetValue(kbdict, CFSTR("KeyBagKeys"));
    CFRetain(kbkeys);
    if (kbkeys == NULL) {
        syslog(LOG_ERR, "FAIL: KeyBagKeys not found\n");
        return -1;
    }
    syslog(LOG_ERR, "Got keys from system keybag\n");
    
    syslog(LOG_ERR, "Writing blobs out to a file");
    //write_file("kbblob.bin", CFDataGetBytePtr(kbkeys), CFDataGetLength(kbkeys));
    syslog(LOG_ERR, "Blobs written");
    
    syslog(LOG_ERR, "Asking AppleKeyStore to parse our keybag blobs");
    KeyBag* kb = AppleKeyStore_parseBinaryKeyBag(kbkeys);
    if (kb == NULL) {
        syslog(LOG_ERR, "FAIL: AppleKeyStore_parseBinaryKeyBag\n");
        return -1;
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
    saveKeybagInfos(kbkeys, kb, key835, passcode, passcodeKey, classKeys);
    CFRelease(classKeys);
    
    return 0;
}

