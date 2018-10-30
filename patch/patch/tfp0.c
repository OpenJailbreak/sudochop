//
//  tfp0.c
//  chop
//
//  Created by Joshua Hill on 1/18/14.
//
//

#include <stdio.h>

#include <syslog.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>

#include "tfp0.h"

void tfp0_kernel_exploit(void* kernel) {
    
}

mach_port_t tfp0_get_port() {
	mach_port_t kernel_task = 0;
	if (!kernel_task) {
		if (task_for_pid(mach_task_self(), 0, &kernel_task) != KERN_SUCCESS) {
			syslog(LOG_WARNING, "Unable to run task_for_pid for kernel!\n");
		}
	}
	return kernel_task;
}

int tfp0_check_map(vm_address_t address) {
	kern_return_t err;
	mach_port_t kernel_task = tfp0_get_port();
	vm_address_t addr = address;
	vm_size_t size;
	int vminfo[VM_REGION_BASIC_INFO_COUNT_64];
	mach_msg_type_number_t len = sizeof(vminfo);
	mach_port_t obj;
    
	err = vm_region_64(kernel_task, &addr, &size, VM_REGION_BASIC_INFO,
                       (vm_region_info_t) & vminfo, &len, &obj);
	if (err != KERN_SUCCESS)
		return 0;
    
	if (addr <= address && address < (addr + size))
		return -1;
    
	return 0;
}

vm_size_t tfp0_kernel_read(vm_address_t addr, void* buffer, vm_size_t size) {
	kern_return_t err = 0;
	mach_port_t kernel_task = tfp0_get_port();
    
	while (size > 0) {
		vm_size_t to_read = (size > 0x800) ? 0x800 : size;
		mach_msg_type_number_t count = (unsigned int) to_read;
        
		if (!tfp0_check_map(addr)) {
			return -1;
        }
        
		err = vm_read_overwrite(kernel_task, addr, to_read, (vm_address_t) buffer, &count);
        
		if (err != KERN_SUCCESS) {
			syslog(LOG_WARNING, "vm_read(0x%lx, 0x%lx) failed\n", addr, to_read);
			return err;
            
		} else if (count != to_read) {
			syslog(LOG_WARNING, "vm_read(0x%lx, 0x%lx) short - 0x%x < 0x%lx\n", addr, to_read, count, to_read);
		}
        
		addr += to_read;
		size -= to_read;
		buffer = ((uint8_t*) buffer) + to_read;
	}
    
	return err;
}

vm_size_t tfp0_kernel_write(vm_address_t addr, void* buffer, vm_size_t size) {
	kern_return_t err = 0;
	mach_port_t task = tfp0_get_port();
    
	if (!tfp0_check_map(addr)) {
		return -1;
    }
    
	err = vm_write(task, addr, (vm_address_t) buffer, size);
    
	if (err != KERN_SUCCESS) {
		syslog(LOG_WARNING, "vm_write(0x%lx, 0x%lx) failed\n", addr, size);
	}
    
	return err;
}

vm_address_t tfp0_kernel_get(vm_address_t address) {
    void* val = 0;
	tfp0_kernel_read(address, &val, sizeof(void*));
	return val;
}

void tfp0_kernel_set(vm_address_t address, void* value) {
    tfp0_kernel_write(address, &value, sizeof(void*));
}