//
//  main.c
//  kaslr_slide
//
//  Created by Joshua Hill on 1/7/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <mach/mach.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/uio.h>

#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

#include "kernel.h"

io_service_t devtree_get_service(const char *name) {
    CFMutableDictionaryRef matching;
    io_service_t service = 0;
    
    matching = IOServiceMatching(name);
    if(matching == NULL) {
        return 0;
    }
    
    while(!service) {
        CFRetain(matching);
        service = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
        if(service) break;
        
        sleep(1);
        CFRelease(matching);
    }
    
    CFRelease(matching);
    return service;
}

vm_address_t devtree_get_bootargs() {
    void* bootargs = 0;
    unsigned char buf[sizeof(unsigned long)*4];
    io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceNameMatching("device-tree"));
    if(service) {
        CFDataRef macData = IORegistryEntryCreateCFProperty(service, CFSTR("IOPlatformArgs"), kCFAllocatorDefault, 0);
        if(macData != NULL) {
            CFDataGetBytes(macData, CFRangeMake(0,sizeof(buf)), buf);
            
            bootargs = *(unsigned long*)&buf[sizeof(unsigned long)];
            
            CFRelease(macData);
            IOObjectRelease(service);
        }
        IOObjectRelease(service);
    }
    return bootargs;
}

vm_address_t devtree_get_base(kernel_t* kernel) {
    syslog(LOG_ERR, "Entering devtree_get_base");
    void* bootargs = devtree_get_bootargs();
    syslog(LOG_ERR, "Found Boot-Args at %p", bootargs);
    vm_address_t kernbase = kernel->get(bootargs+4);
	if (kernel->get(kernbase) == 0xFEEDFACE) {
		syslog(LOG_ERR, "Hello Kernel!! :-) 0x%08lx", kernel->get(kernbase));
        syslog(LOG_ERR, "0x%08lx", kernbase);
	} else {
		syslog(LOG_ERR, "Goodbye Kernel :-( 0x%08lx", kernel->get(kernbase));
	}
    
	return kernbase+0x1000;
}

vm_offset_t devtree_get_slide(kernel_t* kernel) {
    return 0;
}

