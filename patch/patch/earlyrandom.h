//
//  earlyrandom.h
//  patch
//
//  Created by Joshua Hill on 7/22/14.
//
//

#ifndef patch_earlyrandom_h
#define patch_earlyrandom_h

#include <mach/vm_map.h>

#include "kernel.h"

vm_offset_t earlyrandom_get_slide(kernel_t* kernel);
vm_address_t earlyrandom_get_base(kernel_t* kernel);

#endif
