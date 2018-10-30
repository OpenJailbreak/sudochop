/**
 * libmacho-1.0 - segment.h
 * Copyright (C) 2013 Crippy-Dev Team
 * Copyright (C) 2010-2013 Joshua Hill
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef MACHO_SEGMENT_H_
#define MACHO_SEGMENT_H_

#include "section.h"

typedef struct macho_segment_cmd_t {
	uint32_t cmd;
	uint32_t cmdsize;
	char segname[16];
	uint32_t vmaddr;
	uint32_t vmsize;
	uint32_t fileoff;
	uint32_t filesize;
	uint32_t maxprot;
	uint32_t initprot;
	uint32_t nsects;
	uint32_t flags;
} macho_segment_cmd_t;

typedef struct macho_segment_t {
	char* name;
	uint32_t size;
	uint32_t offset;
	uint32_t address;
	unsigned char* data;
	uint32_t section_count;
	macho_section_t** sections;
	macho_segment_cmd_t* command;
} macho_segment_t;

/*
 * Mach-O Segment Functions
 */
macho_segment_t* macho_segment_create();
macho_segment_t* macho_segment_load(unsigned char* data, uint32_t offset);
macho_section_t* macho_segment_get_section(macho_segment_t* segment, const char* section);
void macho_segment_debug(macho_segment_t* segment);
void macho_segment_free(macho_segment_t* segment);

/*
 * Mach-O Segment Info Functions
 */
macho_segment_cmd_t* macho_segment_cmd_create();
macho_segment_cmd_t* macho_segment_cmd_load(unsigned char* data, uint32_t offset);
void macho_segment_cmd_debug(macho_segment_cmd_t* cmd);
void macho_segment_cmd_free(macho_segment_cmd_t* cmd);

#endif /* MACHO_SEGMENT_H_ */
