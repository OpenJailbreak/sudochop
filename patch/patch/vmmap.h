//
//  vmmap.h
//  chop
//
//  Created by Joshua Hill on 1/19/14.
//
//

#ifndef chop_vmmap_h
#define chop_vmmap_h

#include <mach/vm_map.h>

#include "kernel.h"

#if __LP64__
#define ADDR "%16lx"
#define IMAGE_OFFSET 0x2000
#else
#define ADDR "%8x"
#define IMAGE_OFFSET 0x1000
#endif

vm_offset_t vmmap_get_slide(kernel_t* kernel);
vm_address_t vmmap_get_base(kernel_t* kernel);

#endif
