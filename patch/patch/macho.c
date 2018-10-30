/**
 * libmacho-1.0 - macho.c
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//#define _DEBUG
#include "debug.h"

#include "macho.h"
#include "symtab.h"
#include "section.h"

macho_t* macho_create(void);
macho_header_t* macho_header_create(void);

/*
 * Mach-O Functions
 */
macho_t* macho_create() {
	macho_t* macho = (macho_t*) malloc(sizeof(macho_t));
	if (macho) {
		memset(macho, '\0', sizeof(macho_t));
	}
	return macho;
}

macho_t* macho_load(unsigned char* data, uint32_t size) {
	macho_t* macho = NULL;

	macho = macho_create();
	if (macho) {
		macho->offset = 0;
		macho->data = data;
		macho->size = size;
		macho->symtab_count = 0;
		macho->segment_count = 0;

		debug("Loading Mach-O header\n");
		macho->header = macho_header_load(macho);
		if (macho->header == NULL) {
			error("Unable to load Mach-O header information\n");
			macho_free(macho);
			return NULL;
		}
		macho->offset += sizeof(macho_header_t);

		debug("Loading Mach-O commands\n");
		macho->command_count = macho->header->ncmds;
		macho->commands = macho_commands_load(macho);
		if (macho->commands == NULL) {
			error("Unable to parse Mach-O load commands\n");
			macho_free(macho);
			return NULL;
		}

		debug("Loading Mach-O segments\n");
		macho->segments = macho_segments_load(macho);
		if (macho->segments == NULL) {
			error("Unable to parse Mach-O segment commands\n");
			macho_free(macho);
			return NULL;
		}

        /*
		debug("Loading Mach-O symtabs\n");
		macho->symtabs = macho_symtabs_load(macho);
		if (macho->symtabs == NULL) {
			error("Unable to parse Mach-O symtab commands\n");
			macho_free(macho);
			return NULL;
		}
         */
	}

	return macho;
}

macho_t* macho_open(const char* path) {
	int err = 0;
	uint32_t size = 0;
	macho_t* macho = NULL;
	unsigned char* data = NULL;

    /*
	if (path) {
		debug("Reading Mach-O file from path\n");
		err = file_read(path, &data, &size);
		if (err < 0 || size == 0) {
			error("Unable to read Mach-O file\n");
			macho_free(macho);
			return NULL;
		}

		debug("Creating Mach-O object from file\n");
		macho = macho_load(data, size);
		if (macho == NULL) {
			error("Unable to load Mach-O file\n");
			return NULL;
		}
	}
     */
    
	return macho;
}

uint32_t macho_lookup(macho_t* macho, const char* sym) {
	int i = 0;
	int j = 0;
	nlist* nl = NULL;
	macho_symtab_t* symtab = NULL;
	for (i = 0; i < macho->symtab_count; i++) {
		symtab = macho->symtabs[i];
		for (j = 0; j < symtab->nsyms; j++) {
			nl = &symtab->symbols[j];
			if (nl->n_un.n_name != NULL) {
				if (strcmp(sym, nl->n_un.n_name) == 0) {
					return nl->n_value;
				}
			}
		}
	}
	return 0;
}

macho_segment_t* macho_get_segment(macho_t* macho, const char* segment) {
	int i = 0;
	for (i = 0; i < macho->segment_count; i++) {
		macho_segment_t* seg = macho->segments[i];
		if(seg) {
			debug("Found segment %s\n", seg->name);
			if (strcmp(seg->name, segment) == 0) {
				return seg;
			}
		} else {
			debug("Invalid segment!!\n");
		}
	}
	return NULL;
}

macho_section_t* macho_get_section(macho_t* macho, const char* segment,
		const char* section) {
	int i = 0;
	macho_section_t* sect = NULL;
	macho_segment_t* seg = macho_get_segment(macho, segment);
	if (seg) {
		for (i = 0; i < seg->section_count; i++) {
			sect = seg->sections[i];
			if(sect && sect->name) {
				if (strcmp(sect->name, section) == 0) {
					return sect;
				}
			}
		}
	}
	return NULL;
}

void macho_list_symbols(macho_t* macho,
		void (*print_func)(const char*, uint32_t, void*), void* userdata) {
	int i = 0;
	int j = 0;
	nlist* nl = NULL;
	macho_symtab_t* symtab = NULL;
	for (i = 0; i < macho->symtab_count; i++) {
		symtab = macho->symtabs[i];
		for (j = 0; j < symtab->nsyms; j++) {
			nl = &symtab->symbols[j];
			if ((nl->n_un.n_name != NULL) && (nl->n_value != 0)) {
				print_func(nl->n_un.n_name, nl->n_value, userdata);
			}
		}
	}
}

void macho_debug(macho_t* macho) {
	if (macho) {
		debug("Mach-O:\n");
		if (macho) {
			macho_header_debug(macho);
			macho_commands_debug(macho);
			macho_segments_debug(macho);
			//macho_symtabs_debug(macho);
		}
		debug("\n");
	}
}

void macho_free(macho_t* macho) {
	if (macho) {
		if (macho->header) {
			macho_header_free(macho->header);
			macho->header = NULL;
		}
		if (macho->commands) {
			macho_commands_free(macho->commands);
			macho->commands = NULL;
		}

		if (macho->segments) {
			macho_segments_free(macho->segments);
			macho->segments = NULL;
		}
		if (macho->symtabs) {
			macho_symtabs_free(macho->symtabs);
			macho->symtabs = NULL;
		}

		if (macho->data) {
			macho->size = 0;
			macho->offset = 0;
			macho->data = NULL;
		}

		free(macho);
	}
}

/*
 * Mach-O Header Functions
 */
macho_header_t* macho_header_create() {
	macho_header_t* header = (macho_header_t*) malloc(sizeof(macho_header_t));
	if (header) {
		memset(header, '\0', sizeof(macho_header_t));
	}
	return header;
}

macho_header_t* macho_header_load(macho_t* macho) {
	uint32_t size = 0;
	uint32_t offset = 0;
	unsigned char* data = NULL;
	macho_header_t* header = NULL;
	if (macho) {
		offset = 0;
		data = macho->data;
		size = macho->size;
		header = macho_header_create();
		if (header) {
			memcpy(header, &data[offset], sizeof(macho_header_t));
			if (header->magic != MACHO_MAGIC_32) {
				error("Unknown filetype\n");
				return NULL;
			}
		}
	}
	return header;
}

void macho_header_debug(macho_t* macho) {
	macho_header_t* header = NULL;

	header = macho->header;
	if (header) {
		debug("\tHeader:\n");
		debug("\t\t     magic = 0x%08x\n", header->magic);
		debug("\t\t   cputype = 0x%08x\n", header->cputype);
		debug("\t\tcpusubtype = 0x%08x\n", header->cpusubtype);
		debug("\t\t  filetype = 0x%08x\n", header->filetype);
		debug("\t\t     ncmds = 0x%08x\n", header->ncmds);
		debug("\t\tsizeofcmds = 0x%08x\n", header->sizeofcmds);
		debug("\t\t     flags = 0x%08x\n", header->flags);
		debug("\t\n");
	}
}

void macho_header_free(macho_header_t* header) {
	if (header) {
		free(header);
	}
}

int macho_handle_command(macho_t* macho, macho_command_t* command) {
	int ret = 0;
	if (macho) {
		// If load command is a segment command, then load a segment
		//   if a symbol table, then load a symbol table... etc...
		switch (command->info->cmd) {
		case MACHO_CMD_SEGMENT:  // segment of this file to be mapped
		{
			debug("Found segment command\n");
			macho_segment_t* seg = macho_segment_load(macho->data,
					command->offset);
			if (seg) {
				macho->segments[macho->segment_count] = seg;
				debug("Segment has %d sections\n", seg->section_count);

				macho->segment_count++;
			} else {
				error("Could not load segment at offset 0x%x\n",
						command->offset);
			}
		}
			break;
		case MACHO_CMD_SYMTAB:  // link-edit stab symbol table info
		{
			debug("Found symtab command\n");
			macho_symtab_t* symtab = macho_symtab_load(
					macho->data + command->offset, macho->data);
			if (symtab) {
				macho->symtabs[macho->symtab_count++] = symtab;
			} else {
				error("Could not load symtab at offset 0x%x\n",
						command->offset);
			}
		}
			break;
		default:
			ret = -1;
			break;
		}
	}
	return ret;
}

/*
 * Mach-O Commands Functions
 */
macho_command_t** macho_commands_create(uint32_t count) {
	// TODO: Check for integer overflow here
	uint32_t size = (count + 1) * sizeof(macho_command_t*);
	macho_command_t** commands = (macho_command_t**) malloc(size);
	if (commands) {
		memset(commands, '\0', size);
	}
	return commands;
}

macho_command_t** macho_commands_load(macho_t* macho) {
	int i = 0;
	uint32_t count = 0;
	macho_command_t** commands = NULL;
	if (macho) {
		count = macho->command_count;
		debug("Creating Mach-O commands array\n");
		commands = macho_commands_create(count);
		if (commands == NULL) {
			error("Unable to create Mach-O commands array\n");
			return NULL;
		}

		debug("Loading Mach-O commands array\n");
		for (i = 0; i < count; i++) {
			debug("Loading Mach-O command %d from offset 0x%x\n", i, macho->offset);
			commands[i] = macho_command_load(macho->data, macho->offset);
			if (commands[i] == NULL) {
				error("Unable to parse Mach-O load command\n");
				macho_commands_free(commands);
				return NULL;
			}
			macho->offset += commands[i]->size;
		}
	}
	return commands;
}

void macho_commands_debug(macho_t* macho) {
	int i = 0;
	macho_command_t* command = NULL;
	macho_command_t** commands = NULL;

	commands = macho->commands;
	if (commands) {
		debug("\tCommands:\n");
		for(i = 0; i < macho->command_count; i++) {
			command = commands[i];
			if(command) {
				macho_command_debug(command);
			}
		}debug("\t\n");
	}
}

void macho_commands_free(macho_command_t** commands) {
	int i = 0;
	if (commands) {
		while (commands[i] != NULL) {
			macho_command_free(commands[i]);
			commands[i] = NULL;
			i++;
		}
		free(commands);
	}
}

/*
 * Mach-O Segments Functions
 */
macho_segment_t** macho_segments_create(uint32_t count) {
	if (count == 0)
		return NULL;
	// TODO: Check for integer overflow here
	int size = (count + 1) * sizeof(macho_segment_t*);
	macho_segment_t** segments = (macho_segment_t**) malloc(size);
	if (segments) {
		memset(segments, '\0', size);
	}
	return segments;
}

macho_segment_t** macho_segments_load(macho_t* macho) {
	int i = 0;
	int j = 0;
	uint32_t count = 0;
	uint32_t offset = 0;
	macho_segment_t* segment = NULL;
	macho_segment_t** segments = NULL;
	if (macho) {
		debug("Searching for segment commands\n");
		for (i = 0; i < macho->command_count; i++) {
			if (macho->commands[i]->cmd == MACHO_CMD_SEGMENT) {
				count++;
			}
		} debug("Found %d segment commands\n", count);
		macho->segment_count = count;


		debug("Creating Mach-O segments array\n");
		segments = macho_segments_create(count);
		if (segments == NULL) {
			error("Unable to create Mach-O segment array\n");
			return NULL;
		}

		debug("Loading Mach-O segments\n");
		for (i = 0; i < macho->command_count; i++) {
			if (macho->commands[i]->cmd == MACHO_CMD_SEGMENT) {
				macho_command_t* command = macho->commands[i];
				segment = macho_segment_load(macho->data, command->offset);
				if(segment == NULL) {
					error("Unable to load Mach-O segment\n");
					return NULL;
				}
				debug("Loaded in segment %s\n", segment->name);
				segments[j++] = segment;

				macho_section_t* section = NULL;
				macho_section_t** sections = NULL;
				segment->sections = macho_sections_load(macho, segment);
				offset = command->offset + sizeof(macho_segment_cmd_t);
			}
		}
	}

	return segments;
}

void macho_segments_debug(macho_t* macho) {
	int i = 0;
	macho_segment_t* segment = NULL;
	macho_segment_t** segments = NULL;

	if(macho) {
		segments = macho->segments;
		if(segments) {
			debug("\tSegments:\n");
			 for(i = 0; i < macho->segment_count; i++) {
				 segment = segments[i];
				 if(segment) {
					 macho_segment_debug(segment);
				 }
			}debug("\n");
		}
	}
}

void macho_segments_free(macho_segment_t** segments) {
	if (segments) {
		int i = 0;
		while (segments[i]) {
			macho_segment_free(segments[i]);
			i++;
		}
		free(segments);
	}
}

/*
 * Mach-O Symtab Functions
 */
macho_symtab_t** macho_symtabs_create(uint32_t count) {
	if (count == 0)
		return NULL;
	int size = (count + 1) * sizeof(macho_symtab_t*);
	macho_symtab_t** symtabs = (macho_symtab_t**) malloc(size);
	if (symtabs) {
		memset(symtabs, '\0', size);
	}
	return symtabs;
}

macho_symtab_t** macho_symtabs_load(macho_t* macho) {
	int i = 0;
	uint32_t count = 0;
	uint32_t offset = 0;
	macho_symtab_t* symtab = NULL;
	macho_symtab_t** symtabs = NULL;
	if (macho) {
		debug("Searching for symtab commands\n");
		for (i = 0; i < macho->command_count; i++) {
			if (macho->commands[i]->cmd == MACHO_CMD_SYMTAB) {
				count++;
			}
		} debug("Found %d symtab commands\n", count);
		macho->symtab_count = count;

		debug("Creating Mach-O symtabs array\n");
		symtabs = macho_symtabs_create(count);
		if (symtabs == NULL) {
			error("Unable to create Mach-O symtab array\n");
			return NULL;
		}

		debug("Loading Mach-O symtabs\n");
		for (i = 0; i < macho->command_count; i++) {
			if (macho->commands[i]->cmd == MACHO_CMD_SYMTAB) {
				offset =  macho->commands[i]->offset;
				symtab = macho_symtab_load(macho->data, offset);
				if(symtab == NULL) {
					error("Unable to load Mach-O symtab\n");
					return NULL;
				}
				symtabs[i] = symtab;
			}
		}
	}

	return symtabs;
}

void macho_symtabs_debug(macho_t* macho) {
	int i = 0;
	macho_symtab_t* symtab = NULL;
	macho_symtab_t** symtabs = NULL;

	if(macho) {
		if(macho->symtabs) {
			symtabs = macho->symtabs;
			debug("\tSymtabs:\n");
			for(i = 0; i < macho->symtab_count; i++) {
				symtab = macho->symtabs[i];
				macho_symtab_debug(symtab);
				symtab = NULL;
			}
			debug("\n");
		}
	}
}

void macho_symtabs_free(macho_symtab_t** symtabs) {
	if (symtabs) {
		int i = 0;
		while (symtabs[i]) {
			macho_symtab_free(symtabs[i]);
			i++;
		}
		free(symtabs);
	}
}

/*
 * Mach-O Sections Functions
 */
macho_section_t** macho_sections_create(uint32_t count) {
	// TODO: Check for integer overflow here
	unsigned int size = (count + 1) * sizeof(macho_section_t*);
	macho_section_t** sections = (macho_section_t**) malloc(size);
	if (sections) {
		memset(sections, '\0', size);
	}
	return sections;
}

macho_section_t** macho_sections_load(macho_t* macho, macho_segment_t* segment) {
	int i = 0;
	uint32_t offset = 0;
	macho_section_t* section = NULL;
	macho_section_t** sections = NULL;

	if (macho && segment) {
		debug("Creating section array for segment\n");
		sections = macho_sections_create(segment->section_count);
		if (sections == NULL) {
			error("Unable to create section array for segment\n");
			return NULL;
		}

		offset = segment->offset + sizeof(macho_segment_cmd_t);
		for (i = 0; i < segment->section_count; i++) {
			debug("Loading section %d\n", i);
			sections[i] = macho_section_load(macho->data, offset);
			offset += sizeof(macho_section_info_t);
		}
	}
	return sections;
}

void macho_sections_debug(macho_section_t** sections) {
	debug("\tSections:\n");debug("\t\n");
}

void macho_sections_free(macho_section_t** sections) {
	// TODO: Loop through and free each item
	if (sections) {
		free(sections);
	}
}
