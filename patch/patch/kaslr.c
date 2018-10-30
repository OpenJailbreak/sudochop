//
//  kaslr.c
//  chop
//
//  Created by Joshua Hill on 1/18/14.
//
//

#include <stdio.h>
#include <stdlib.h>

#include "vmmap.h"
#include "devtree.h"

#include "kaslr.h"

kaslr_t* kaslr_init() {
    kaslr_t* kaslr = (kaslr_t*) malloc(sizeof(kaslr_t));
    if(kaslr) {
        memset(kaslr, '\0', sizeof(kaslr_t));
        //if(tfp0) {
        
        // Defaulting to tfp0 method for now
        kaslr->base = vmmap_get_base;
        kaslr->slide = vmmap_get_slide;
        
        // TODO: Chicken and egg problem here since we need to read bootargs from kernel
        //} else {
        //    kaslr->base = devtree_get_base;
        //    kaslr->slide = devtree_get_slide;
        //}
    }
    return kaslr;
}

void kaslr_free(kaslr_t* kaslr) {
    if(kaslr) {
        free(kaslr);
        kaslr = NULL;
    }
}