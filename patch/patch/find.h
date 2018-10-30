//
//  find_xref.h
//  SudoChop
//
//  Created by Joshua Hill on 12/12/12.
//
//

#ifndef __chop__find_xref__
#define __chop__find_xref__

#include <stdio.h>
#include <mach/vm_map.h>
#include <stdint.h>
#include <string.h>

#include "kernel.h"

/*
 * ARM page bits for L1 sections.
 */
#define L1_SHIFT            20            /* log2(1MB) */

#define L1_SECT_PROTO        (1 << 1)        /* 0b10 */

#define L1_SECT_B_BIT        (1 << 2)
#define L1_SECT_C_BIT        (1 << 3)

#define L1_SECT_SORDER       (0)            /* 0b00, not cacheable, strongly ordered. */
#define L1_SECT_SH_DEVICE    (L1_SECT_B_BIT)
#define L1_SECT_WT_NWA       (L1_SECT_C_BIT)
#define L1_SECT_WB_NWA       (L1_SECT_B_BIT | L1_SECT_C_BIT)
#define L1_SECT_S_BIT        (1 << 16)

#define L1_SECT_AP_URW       (1 << 10) | (1 << 11)
#define L1_SECT_PFN(x)       (x & 0xFFF00000)

#define L1_SECT_DEFPROT      (L1_SECT_AP_URW)
#define L1_SECT_DEFCACHE     (L1_SECT_SORDER)

#define L1_PROTO_TTE(paddr)  (L1_SECT_PFN(paddr) | L1_SECT_S_BIT | L1_SECT_DEFPROT | L1_SECT_DEFCACHE | L1_SECT_PROTO)

#define PFN_SHIFT            2
#define TTB_OFFSET(vaddr)    ((vaddr >> L1_SHIFT) << PFN_SHIFT)

/*
 * RAM physical base begin.
 */
#define S5L8930_PHYS_OFF    0x40000000
#define S5L8940_PHYS_OFF    0x80000000        /* Note: RAM base is identical for 8940-8955. */

#define PHYS_OFF            S5L8940_PHYS_OFF

/*
 * Shadowmap begin and end. 15MB of shadowmap is enough for the kernel.
 * We don't need to invalidate unified D/I TLB or any cache lines
 * since the kernel is mapped as writethrough memory, and these
 * addresses are guaranteed to not be translated.
 * (Accesses will cause segmentation faults due to failure on L1 translation.)
 *
 * Clear the shadowmappings when done owning the kernel.
 *
 * 0x7ff0'0000 is also below the limit for vm_read and such, so that's also *great*.
 * (2048 bytes)
 */
#define SHADOWMAP_BEGIN          0x7f000000
#define SHADOWMAP_END            0x7ff00000
#define SHADOWMAP_GRANULARITY    0x00100000

#define SHADOWMAP_SIZE_BYTES    (SHADOWMAP_END - SHADOWMAP_BEGIN)

#define SHADOWMAP_BEGIN_OFF     TTB_OFFSET(SHADOWMAP_BEGIN)
#define SHADOWMAP_END_OFF       TTB_OFFSET(SHADOWMAP_END)
#define SHADOWMAP_SIZE          (SHADOWMAP_END_OFF - SHADOWMAP_BEGIN_OFF)

#define SHADOWMAP_BEGIN_IDX     (SHADOWMAP_BEGIN_OFF >> PFN_SHIFT)
#define SHADOWMAP_END_IDX       (SHADOWMAP_END_OFF >> PFN_SHIFT)

#define TTB_SIZE                4096
#define DEFAULT_KERNEL_SLIDE    0x80000000

vm_address_t find_tfp(kernel_t* kernel);
//vm_address_t find_aes(kernel_t* kernel);
uint32_t find_aes(uint32_t region, uint8_t* kdata, size_t ksize);
vm_address_t find_dbg(kernel_t* kernel);
vm_address_t find_iof(kernel_t* kernel);
vm_address_t find_tlb(kernel_t* kernel);

// Helper gadget.
uint32_t find_memmove(uint32_t region, uint8_t* kdata, size_t ksize);

// Use for write-anywhere gadget.
uint32_t find_str_r1_r2_bx_lr(uint32_t region, uint8_t* kdata, size_t ksize);

// Helper gadget for changing page tables / patching.
uint32_t find_flush_dcache(uint32_t region, uint8_t* kdata, size_t ksize);

// Helper gadget for changing page tables.
uint32_t find_invalidate_tlb(uint32_t region, uint8_t* kdata, size_t ksize);

// This points to kernel_pmap. Use that to change the page tables if necessary.
uint32_t find_pmap_location(uint32_t region, uint8_t* kdata, size_t ksize);

// Write 0 here.
uint32_t find_proc_enforce(uint32_t region, uint8_t* kdata, size_t ksize);

// Write 1 here.
uint32_t find_cs_enforcement_disable_amfi(uint32_t region, uint8_t* kdata, size_t ksize);

// Write 1 here.
uint32_t find_cs_enforcement_disable_kernel(uint32_t region, uint8_t* kdata, size_t ksize);

// Change this to non-zero.
uint32_t find_i_can_has_debugger_1(uint32_t region, uint8_t* kdata, size_t ksize);

// Change this to what you want the value to be (non-zero appears to work).
uint32_t find_i_can_has_debugger_2(uint32_t region, uint8_t* kdata, size_t ksize);

// NOP out the conditional branch here.
uint32_t find_vm_map_enter_patch(uint32_t region, uint8_t* kdata, size_t ksize);

// Change the conditional branch here to an unconditional branch.
uint32_t find_vm_map_protect_patch(uint32_t region, uint8_t* kdata, size_t ksize);

// Change the conditional branch here to an unconditional branch.
uint32_t find_tfp0_patch(uint32_t region, uint8_t* kdata, size_t ksize);

// Write this with a jump to the sandbox hook, then write a trampoline back to just after the jump you wrote here. Sandbox hook should look at the path in *(r3 + 0x14) and force
// it to be allowed if it is outside of /private/var/mobile, or inside of /private/var/mobile/Library/Preferences but not /private/var/mobile/Library/Preferences/com.apple*
// To force it to allow, *r0 = 0 and *(r0 + 0x4) = 0x18. If not, just call the original function via the trampoline.
uint32_t find_sb_patch(uint32_t region, uint8_t* kdata, size_t ksize);

// Utility function, necessary for the sandbox hook.
uint32_t find_vn_getpath(uint32_t region, uint8_t* kdata, size_t ksize);

// Utility function, necessary for the sandbox hook.
uint32_t find_memcmp(uint32_t region, uint8_t* kdata, size_t ksize);

// Dereference this, add 0x38 to the resulting pointer, and write whatever boot-args are suitable to affect kern.bootargs.
uint32_t find_p_bootargs(uint32_t region, uint8_t* kdata, size_t ksize);

// This gets the zone_page_table array in osfmk/kern/zalloc.c. Useful for diagnosing problems with the zone allocator.
uint32_t find_zone_page_table(uint32_t region, uint8_t* kdata, size_t ksize);

// Function to free leaked ipc_kmsg objects
uint32_t find_ipc_kmsg_destroy(uint32_t region, uint8_t* kdata, size_t ksize);

// Function to find the syscall 0 function pointer. Used to modify the syscall table to call our own code.
uint32_t find_syscall0(uint32_t region, uint8_t* kdata, size_t ksize);

// Function used to free any dead ports we find to clean up after memory leak.
uint32_t find_io_free(uint32_t region, uint8_t* kdata, size_t ksize);

// Function used to find IOLog for printing debug messages
uint32_t find_IOLog(uint32_t region, uint8_t* kdata, size_t ksize);

#endif /* defined(__chop__find_xref__) */
