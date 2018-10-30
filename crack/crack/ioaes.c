//
//  main.c
//  ioaes
//
//  Created by Joshua Hill on 1/7/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "util.h"
#include "IOKit.h"
#include "IOAESAccelerator.h"

int ioaes(int argc, char* argv[]) {
    uint32_t key = 0;
    if(argc == 2) {
        key = strtoul(argv[1], NULL, 0);
    } else {
        fprintf(stderr, "Usage: ioaes [ 0x835 | 0x89A | 0x89B ]\n");
        return -1;
    }
    
    syslog(LOG_ERR, "Pwning UID Keys!!!");
    if(key == 0x835) {
        printf("%s\n", hex2str(IOAES_key835(), 16));
        syslog(LOG_ERR, "KEY 0x835: %s", hex2str(IOAES_key835(), 16));
        
    } else if(key == 0x89A) {
        printf("%s\n", hex2str(IOAES_key89A(), 16));
        syslog(LOG_ERR, "KEY 0x835: %s", hex2str(IOAES_key89A(), 16));
        
    } else if(key == 0x89B) {
        printf("%s\n", hex2str(IOAES_key89B(), 16));
        syslog(LOG_ERR, "KEY 0x835: %s", hex2str(IOAES_key89B(), 16));
        
    } else {
        fprintf(stderr, "Usage: ioaes [ 0x835 | 0x89A | 0x89B ]\n");
        syslog(LOG_ERR, "Invalid usage");
        return -1;
    }
    
	return 0;
}

