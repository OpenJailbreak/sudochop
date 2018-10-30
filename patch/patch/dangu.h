//
//  dangu.h
//  patch
//
//  Created by Joshua Hill on 7/22/14.
//
//

#ifndef patch_dangu_h
#define patch_dangu_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <IOKit/IOKitLib.h>
#include <unistd.h>

int dangu_exploit(void* kernel);

vm_address_t dangu_kernel_get(vm_address_t address);
void dangu_kernel_set(vm_address_t address, void* value);
vm_size_t dangu_kernel_read(vm_address_t addr, void* buffer, vm_size_t size);
vm_size_t dangu_kernel_write(vm_address_t addr, void* buffer, vm_size_t size);

#endif
