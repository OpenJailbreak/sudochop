//
//  kernel.c
//  chop
//
//  Created by Joshua Hill on 1/18/14.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <syslog.h>

#include "avio.h"
#include "tfp0.h"
#include "kaslr.h"
#include "vmmap.h"
#include "dangu.h"
#include "devtree.h"
#include "m1n0rthreat.h"

#include "kernel.h"

kernel_t* kernel_open() {
    kernel_t* kernel = (kernel_t*)malloc(sizeof(kernel_t));
    if(kernel) {
        memset(kernel, '\0', sizeof(kernel_t));
        //kaslr_init();
        kernel->task = tfp0_get_port();
        if(kernel->task != 0) {
            // tfp0 is patched
            
            // TODO: Implement these later
            kernel->get = tfp0_kernel_get;
            kernel->set = tfp0_kernel_set;
            //kernel->exec = tfp0_kernel_exec;
            kernel->exploit = dangu_exploit;
            
            kernel->read = tfp0_kernel_read;
            kernel->write = tfp0_kernel_write;
            
            // This should really be decided by the base kaslr class
            kernel->base = devtree_get_base(kernel);
            kernel->slide = devtree_get_slide(kernel);
            vmmap_get_base(kernel);
            syslog(LOG_ERR, "Found kernel base at 0x%08lx", kernel->base);
            
        } else {
            // TODO: Need to detect version to choose proper exploit
            // Default to 7.0.4 kernel exploit for now
            
            // TODO: Implement these later
            //kernel->get = m1n0rthreat_kernel_get;
            //kernel->set = m1n0rthreat_kernel_set;
            //kernel->exec = m1n0rthreat_kernel_exec;
            
            //kernel->read = m1n0rthreat_kernel_read;
            //kernel->write = m1n0rthreat_kernel_write;
            
            // AVIO
            //kernel->get = avio_kernel_get;
            //kernel->set = avio_kernel_set;
            //kernel->exec = avio_kernel_exec;
            
            //kernel->read = avio_kernel_read;
            //kernel->write = avio_kernel_write;
            
            // This should really be decided by the base kaslr class
            //kernel->base = devtree_get_base(kernel);
            //kernel->slide = devtree_get_slide(kernel);
            // TODO: After patching tfp0 we might want to reopen
        }
    }
    return kernel;
}

kernel_t* x_kernel_open() {
    kernel_t* kernel = (kernel_t*)malloc(sizeof(kernel_t));
    if(kernel) {
        memset(kernel, '\0', sizeof(kernel_t));
        
    }
    return kernel;
}

void kernel_close(kernel_t* kernel) {
    if(kernel) {
        if(kernel->task) {
            mach_port_deallocate(mach_task_self(), kernel->task);
            kernel->task = 0;
        }
        free(kernel);
        kernel = NULL;
    }
}