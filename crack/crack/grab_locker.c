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
#include "AppleKeyStore.h"
#include "IOAESAccelerator.h"
#include "AppleEffaceableStorage.h"

#include "bsdcrypto/rijndael.h"
#include "bsdcrypto/key_wrap.h"

#include "util.h"

/*
 #define LOCKER_DKEY 0x446B6579
 #define LOCKER_EMF  0x454D4621
 #define LOCKER_BAG1 0x42414731
 #define LOCKER_LWVM 0x4C77564d
 */

/*
 struct EffaceableLocker
 {
 unsigned short magic;   //0x4c6B = "kL"
 unsigned short len;
 unsigned int tag;       //BAG1, EMF, Dkey, DONE
 unsigned char data[1];
 };
 
 struct BAG1Locker
 {
 unsigned int magic;//'BAG1';
 unsigned char iv[16];
 unsigned char key[32];
 };
 */

int grab_locker(uint32_t entry, unsigned char** data, unsigned int* size) {
    int x = 0;
    char* locker = 0;
    uint8_t emf[36]={0};
    uint8_t dkey[40]={0};
    uint8_t lwvm[80]={0};
    uint8_t lockers[960]={0};
    struct BAG1Locker bag1_locker={0};
    
    x = AppleEffaceableStorage__getBytes(lockers, 960);
    syslog(LOG_ERR, "Pwning AppleEffaceableStorage!!!");
    if(strcmp(locker, "BAG1") == 0) {
        AppleEffaceableStorage__getLocker(LOCKER_BAG1, (uint8_t*) &bag1_locker, sizeof(struct BAG1Locker));
        printf("%s\n", hex2str((uint8_t*)&bag1_locker, sizeof(struct BAG1Locker)));
        syslog(LOG_ERR, "BAG1: %s\n", hex2str((uint8_t*)&bag1_locker, sizeof(struct BAG1Locker)));
        
    } else if(strcmp(locker, "LWVM") == 0) {
        AppleEffaceableStorage__getLockerFromBytes(LOCKER_LWVM, lockers, 960, lwvm, 0x50);
        printf("%s\n", hex2str(lwvm, 0x50));
        syslog(LOG_ERR, "LWVM: %s\n", hex2str(lwvm, 0x50));
        
    } else if(strcmp(locker, "EMF") == 0) {
        AppleEffaceableStorage__getLocker(LOCKER_EMF, emf, 36);
        printf("%s\n", hex2str(emf, 36));
        syslog(LOG_ERR, "EMF: %s\n", hex2str(emf, 36));
        
    } else if(strcmp(locker, "DKEY") == 0) {
        AppleEffaceableStorage__getLockerFromBytes(LOCKER_DKEY, lockers, 960, dkey, 40);
        printf("%s\n", hex2str(dkey, 40));
        syslog(LOG_ERR, "Dkey: %s\n", hex2str(dkey, 40));
    } else {
        fprintf(stderr, "Usage: grab_locker [ EMF | DKEY | BAG1 | LWVM ]\n");
        syslog(LOG_ERR, "Invalid usage");
        return -1;
    }
    
	return 0;
}
