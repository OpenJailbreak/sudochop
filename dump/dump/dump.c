//
//  dump.c
//  dump
//
//  Created by Joshua Hill on 1/16/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.

#include <stdio.h>
#include <syslog.h>

#include "disk.h"
#include "dump.h"

void dump_enter(void) __attribute__((constructor));
void dump_leave(void) __attribute__((destructor));

void dump_enter() {
    syslog(LOG_ERR, "++[DUMP]++ Entering dump.dylib");
    int x = disk_dump("/dev/rdisk0s1s2", 2420);
    if(x) {
        syslog(LOG_ERR, "++[DUMP]++ Error dumping data");
    } else {
        syslog(LOG_ERR, "++[DUMP]++ Dump finished");
    }
}

void dump_leave() {
    syslog(LOG_ERR, "++[DUMP]++ Leaving dump.dylib\n");
}