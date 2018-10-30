//
//  patch.h
//  patch
//
//  Created by Joshua Hill on 1/18/14.
//
//

#ifndef patch_patch_h
#define patch_patch_h

#include <mach/vm_map.h>

#include "kernel.h"

#define TTE_OFFSET(ADDR) (((ADDR >> 0x14) & 0xfff) << 2)
#define ADDR_TO_TTE(BASE, ADDR) (BASE + TTE_OFFSET(ADDR))
#define PTE_OFFSET(ADDR) (((ADDR & ~(0xfff00000)) >> 12) << 2)

int patch_tfp(kernel_t* kernel);
int patch_aes(kernel_t* kernel);
int patch_dbg(kernel_t* kernel);
int patch_iof(kernel_t* kernel);
int patch_tlb(kernel_t* kernel, vm_address_t address);

#endif
