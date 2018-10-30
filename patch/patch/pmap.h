//
//  pmap.h
//  patch
//
//  Created by Joshua Hill on 3/27/14.
//
//

#ifndef patch_pmap_h
#define patch_pmap_h

#include <mach/vm_map.h>

vm_address_t pmap_kernel_get(vm_address_t address);
void pmap_kernel_set(vm_address_t address, void* value);
vm_size_t pmap_kernel_read(vm_address_t addr, void* buffer, vm_size_t size);
vm_size_t pmap_kernel_write(vm_address_t addr, void* buffer, vm_size_t size);

#endif
