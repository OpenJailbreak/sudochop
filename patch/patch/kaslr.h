//
//  kaslr.h
//  chop
//
//  Created by Joshua Hill on 1/18/14.
//
//

#ifndef chop_kaslr_h
#define chop_kaslr_h

#include <mach/mach_types.h>

#include "kernel.h"

typedef vm_address_t(*kaslr_get_base)(void);
typedef vm_offset_t(*kaslr_get_slide)(void);

typedef struct {
    kaslr_get_base base;
    kaslr_get_slide slide;
} kaslr_t;

kaslr_t* kaslr_init(kernel_t* kernel);
void kaslr_free(kaslr_t* kaslr);

#endif
