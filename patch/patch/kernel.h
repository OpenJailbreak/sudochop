//
//  kernel.h
//  chop
//
//  Created by Joshua Hill on 1/18/14.
//
//

#ifndef chop_kernel_h
#define chop_kernel_h

#include <stdint.h>

#include <mach/mach.h>

typedef int(*kernel_exploit_t)(void* kernel);
typedef vm_address_t(*kernel_get_t)(vm_address_t address);
typedef void(*kernel_set_t)(vm_address_t address, void* value);
typedef vm_size_t(*kernel_read_t)(vm_address_t address, void* data, vm_size_t size);
typedef vm_size_t(*kernel_write_t)(vm_address_t address, void* data, vm_size_t size);

typedef struct {
    mach_port_t task;
    vm_offset_t slide;
    vm_address_t base;
    vm_address_t pmap;
    
    kernel_get_t get;
    kernel_set_t set;
    
    kernel_read_t read;
    kernel_write_t write;
    kernel_exploit_t exploit;
} kernel_t;

kernel_t* kernel_open();
void kernel_close(kernel_t* kernel);

#endif
