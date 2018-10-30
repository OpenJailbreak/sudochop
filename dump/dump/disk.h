//
//  dump.h
//  chop
//
//  Created by Joshua Hill on 1/18/14.
//
//

#ifndef chop_dump_h
#define chop_dump_h

#include <stdint.h>
#include <sys/ioctl.h>

#define DKIOCGETBLOCKSIZE                     _IOR('d', 24, uint32_t)
#define DKIOCGETBLOCKCOUNT                    _IOR('d', 25, uint64_t)

int disk_dump(const char* device, uint16_t port);
//int disk_flash(char* device);

#endif
