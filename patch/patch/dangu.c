//
//  dangu.c
//  patch
//
//  Created by Joshua Hill on 7/22/14.
//
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <IOKit/IOKitLib.h>
#include <unistd.h>

#include "kernel.h"

int dangu_exploit(void* kernel) {
    kernel_t* kern = (kernel_t*)kernel;
    
    return 0;
}

vm_address_t dangu_kernel_get(vm_address_t address) {
    return 0;
}

void dangu_kernel_set(vm_address_t address, void* value) {
    return;
}

vm_size_t dangu_kernel_read(vm_address_t addr, void* buffer, vm_size_t size) {
    return 0;
}

vm_size_t dangu_kernel_write(vm_address_t addr, void* buffer, vm_size_t size) {
    return 0;
}