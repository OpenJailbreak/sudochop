/**
 * libmacho-1.0 - section.h
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

#ifndef MACHO_SECTION_H_
#define MACHO_SECTION_H_

typedef struct macho_section_info_t {
	char		sectname[16];	/* name of this section */
	char		segname[16];	/* segment this section goes in */
	uint32_t	addr;		/* memory address of this section */
	uint32_t	size;		/* size in bytes of this section */
	uint32_t	offset;		/* file offset of this section */
	uint32_t	align;		/* section alignment (power of 2) */
	uint32_t	reloff;		/* file offset of relocation entries */
	uint32_t	nreloc;		/* number of relocation entries */
	uint32_t	flags;		/* flags (section type and attributes)*/
	uint32_t	reserved1;	/* reserved (for offset or index) */
	uint32_t	reserved2;	/* reserved (for count or sizeof) */
} macho_section_info_t;

typedef struct macho_section_t {
	char* name;
	macho_section_info_t* info;
} macho_section_t;

/*
 * Mach-O Segment Functions
 */
macho_section_t* macho_section_create();
macho_section_t* macho_section_load(unsigned char* data, uint32_t offset);
void macho_section_debug(macho_section_t* section);
void macho_section_free(macho_section_t* section);

/*
 * Mach-O Segment Info Functions
 */
macho_section_info_t* macho_section_info_create();
macho_section_info_t* macho_section_info_load(unsigned char* data, uint32_t offset);
void macho_section_info_debug(macho_section_info_t* info);
void macho_section_info_free(macho_section_info_t* info);

#endif /* MACHO_SECTION_H_ */
