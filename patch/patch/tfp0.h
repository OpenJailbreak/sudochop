//
//  tfp0.h
//  chop
//
//  Created by Joshua Hill on 1/19/14.
//
//

#ifndef chop_tfp0_h
#define chop_tfp0_h

#include <mach/mach.h>

mach_port_t tfp0_get_port();

void tfp0_kernel_exploit(void* kernel);
vm_address_t tfp0_kernel_get(vm_address_t address);
void tfp0_kernel_set(vm_address_t address, void* value);
vm_size_t tfp0_kernel_read(vm_address_t addr, void* buffer, vm_size_t size);
vm_size_t tfp0_kernel_write(vm_address_t addr, void* buffer, vm_size_t size);

#endif
