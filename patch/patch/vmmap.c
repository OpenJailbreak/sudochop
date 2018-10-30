//
//  vmmap.c
//  chop
//
//  Created by Joshua Hill on 1/19/14.
//
//

#include <stdio.h>
#include <syslog.h>

#include <mach/vm_map.h>
#include <mach/host_priv.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>
#include <mach/mach_traps.h>
#include <mach/mach_types.h>

#include "vmmap.h"
#include "kernel.h"

vm_offset_t vmmap_get_slide(kernel_t* kernel) {
    return 0;
}

vm_address_t vmmap_get_base(kernel_t* kernel) {
    vm_size_t size = 0;
    kern_return_t kr = 0;
    task_t kernel_task = 0;
    vm_address_t addr = 0x81200000;
    vm_region_submap_info_data_64_t info;
    mach_msg_type_number_t info_count = VM_REGION_SUBMAP_INFO_COUNT_64;
    unsigned int depth = 0;
    
    syslog(LOG_ERR, "Entering vmmap_get_base");
    kr = task_for_pid(mach_task_self(), 0, &kernel_task);
    if (kr != KERN_SUCCESS) {
        return NULL;
    }
    
    while (1) {
        // get next memory region
        kr = vm_region_recurse_64(kernel_task, &addr, &size, &depth, (vm_region_info_t)&info, &info_count);
        
        if (kr != KERN_SUCCESS) break;
        
        // the kernel maps over a GB of RAM at the address where it maps
        // itself so we use that fact to detect it's position
        if (size > 1024*1024*1024) {
            syslog(LOG_ERR, "Found possible range at 0x%lx with size 0x%lx", addr, size);
            //vm_address_t magic = kernel->get(addr+IMAGE_OFFSET);
            //if(magic == 0xFEEDFACE) {
            //    syslog(LOG_ERR, "Verified base address");
            //    return addr + IMAGE_OFFSET;
            //}
        }
        
        addr += size;
    }
    
    return NULL;
}