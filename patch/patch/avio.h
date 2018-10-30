//
//  avio.h
//  patch
//
//  Created by Joshua Hill on 6/6/14.
//
//

#ifndef patch_avio_h
#define patch_avio_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <IOKit/IOKitLib.h>
#include <unistd.h>
#include <asl.h>

//kern.osversion
#define VALID_VERSION "13D65"

void write_mem_from_av(uint32_t address, uint32_t value);

void close_av_device();

io_connect_t open_av_device();
uint32_t get_address_from_asl();
void set_av_ioservice_addr(uint32_t addr);

uint32_t get_av_ioservice_addr();

void write_mem_from_av(uint32_t address, uint32_t value);

vm_address_t avio_kernel_get(vm_address_t address);
void avio_kernel_set(vm_address_t address, void* value);
vm_size_t avio_kernel_read(vm_address_t addr, void* buffer, vm_size_t size);
vm_size_t avio_kernel_write(vm_address_t addr, void* buffer, vm_size_t size);

#endif
