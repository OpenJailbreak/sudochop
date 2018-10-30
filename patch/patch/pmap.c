//
//  pmap.c
//  patch
//
//  Created by Joshua Hill on 3/27/14.
//
//

#include <stdio.h>
#include <mach/vm_map.h>

#include "pmap.h"

vm_address_t pmap_kernel_get(vm_address_t address) {
    return NULL;
}

void pmap_kernel_set(vm_address_t address, void* value) {
    
}

vm_size_t pmap_kernel_read(vm_address_t addr, void* buffer, vm_size_t size) {
    return 0;
}

vm_size_t pmap_kernel_write(vm_address_t addr, void* buffer, vm_size_t size) {
    return 0;
}