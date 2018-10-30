//
//  devicetree.h
//  chop
//
//  Created by Joshua Hill on 1/19/14.
//
//

#ifndef chop_devicetree_h
#define chop_devicetree_h

#include <stdio.h>
#include <mach/vm_map.h>
#include <mach/mach_types.h>

#include "kernel.h"

vm_address_t devtree_get_bootargs();
vm_offset_t devtree_get_slide(kernel_t* kernel);
vm_address_t devtree_get_base(kernel_t* kernel);


#endif
