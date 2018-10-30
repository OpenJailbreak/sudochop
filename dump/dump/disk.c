//
//  disk.c
//  com.sudo-sec.chop.dump
//
//  Created by Joshua Hill on 1/18/14.
//
//

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "sock.h"
#include "disk.h"

int disk_dump(const char* device, uint16_t port) {
    int x = 0;
    int y = 0;
	int fd = 0;
    uint64_t total = 0;
    uint64_t totalsize = 0;
	uint32_t blocksize = 0;
    uint64_t blocktotal = 0;
    uint64_t blockcount = 0;
	unsigned char buffer[0x2000];
    
	memset(buffer, '\0', 0x2000);
	syslog(LOG_ERR, "++[DISK]++ Openning raw block device\n");
	fd = open(device, O_RDWR);
	if (fd > 0) {
        syslog(LOG_ERR, "++[DISK]++ Successfully opened disk0s1s1 block device");
        
        //blocksize = 0x2000;
        ioctl(fd, DKIOCGETBLOCKSIZE , &blocksize);
        ioctl(fd, DKIOCGETBLOCKCOUNT , &blockcount);
        blockcount /= 2;
		syslog(LOG_ERR, "++[DISK]++ Blocksize is 0x%08x\n", blocksize);
		syslog(LOG_ERR, "++[DISK]++ Blockcount is 0x%llx\n", blockcount);
        
        
        syslog(LOG_ERR, "++[DISK]++ Listening on port %hd", port);
        int s = sock_listen(port);
        if(s > 0) {
            syslog(LOG_ERR, "++[DISK]++ Waiting for client to connect");
            int c = sock_accept(s);
            if(c > 0) {
                syslog(LOG_ERR, "++[DISK]++ Successfully connecteded to client");
                totalsize = blocksize * blockcount;
                sock_send(c, (unsigned char*)&totalsize, sizeof(totalsize));
                
                
                syslog(LOG_ERR, "++[DISK]++ Reading data from block device and writing to socket\n");
                while(blocktotal <= blockcount) {
                    memset(buffer, '\0', 0x2000);
                    x = read(fd, buffer, blocksize);
                    if (x > 0) {
                        y = sock_send(c, buffer, x);
                        if(y != x) {
                            syslog(LOG_ERR, "++[DISK]++ Unable to send same number of bytes\n");
                            break;
                        }
                        total += x;
                        blocktotal++;
                        if(total != 0 && (total % 0x8000000) == 0) {
                            syslog(LOG_ERR, "++[DISK]++ 0x%llx out of 0x%llx blocks written so far\n", blocktotal, blockcount);
                        }
                    } else {
                        syslog(LOG_ERR, "++[DISK]++ Read 0 or less bytes, appear to be finished");
                        //syslog(LOG_ERR, "++[DISK]++ Just to make sure we're not just at a hidden block, let's continue");
                        //total += blocksize;
                        break;
                    }
                }
                
                close(c);
            }
            
            syslog(LOG_ERR, "++[DISK]++ Successfully read and send 0x%llx blocks totalling 0x%llx bytes", blocktotal, total);
            syslog(LOG_ERR, "++[DISK]++ Closing socket");
            close(s);
        }
        
        syslog(LOG_ERR, "++[DISK]++ Closing raw block device\n");
        close(fd);
        fd = 0;
        
    } else {
        syslog(LOG_ERR, "++[DISK]++ Unable to open raw block device for reading: %d", x);
        return -1;
    }
    return 0;
}
