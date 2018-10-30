//
//  debug.c
//  konsole
//
//  Created by Joshua Hill on 7/22/14.
//
//

#include <stdio.h>
#include <syslog.h>

void hexdump(unsigned char* buf, unsigned int len) {
	int i, j;
    unsigned int offset = 0;
    char buffer[0x4000];
    memset(buffer, '\0', 0x4000);
    
	offset += sprintf(&buffer[offset], "0x%08x: ", buf);
	for (i = 0; i < len; i++) {
		if (i % 16 == 0 && i != 0) {
			for (j=i-16; j < i; j++) {
				unsigned char car = buf[j];
				if (car < 0x20 || car > 0x7f) car = '.';
				offset += sprintf(&buffer[offset], "%c", car);
			}
            syslog(LOG_ERR, "%s", buffer);
            offset = 0;
            memset(buffer, '\0', 0x4000);
			offset += sprintf(&buffer[offset], "0x%08x: ", buf+i);
		}
		offset += sprintf(&buffer[offset], "%02x ", buf[i]);
	}
    
	int done = (i % 16);
	int remains = 16 - done;
	if (done > 0) {
		for (j = 0; j < remains; j++) {
			offset += sprintf(&buffer[offset], "   ");
		}
	}
    
	if ((i - done) >= 0) {
		if (done == 0 && i > 0) done = 16;
		for (j = (i - done); j < i; j++) {
			unsigned char car = buf[j];
			if (car < 0x20 || car > 0x7f) car = '.';
			offset += sprintf(&buffer[offset], "%c", car);
		}
	}
    
	syslog(LOG_ERR, "%s", buffer);
    memset(buffer, '\0', 0x4000);
    offset = 0;
}