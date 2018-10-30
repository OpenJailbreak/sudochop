//
//  kshell.c
//  konsole
//
//  Created by Joshua Hill on 7/22/14.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "shell.h"
#include "kernel.h"

static kernel_t* kern = NULL;

int cmd_md(int argc, char* argv[]);
int cmd_mw(int argc, char* argv[]);
int cmd_help(int argc, char* argv[]);
int cmd_exit(int argc, char* argv[]);
int cmd_echo(int argc, char* argv[]);
int cmd_base(int argc, char* argv[]);
int cmd_slide(int argc, char* argv[]);
int cmd_exploit(int argc, char* argv[]);
int cmd_hexdump(int argc, char* argv[]);

static const cmd_info_t commands[] = {
    { "help",    cmd_help,    "Print help message"            },
    { "exit",    cmd_exit,    "Exit kernel shell"             },
    { "echo",    cmd_echo,    "Echo characters to the screen" },
    { "md",      cmd_md,      "Display the value at address"  },
    { "mw",      cmd_mw,      "Write value to address"        },
    { "base",    cmd_base,    "Get kernel base address"       },
    { "slide",   cmd_slide,   "Get kernel ASLR slide"         },
    { "hexdump", cmd_hexdump, "Dump memory from kernel"       },
    { "exploit", cmd_exploit, "Test currently loaded exploit" },
    { 0, 0, 0 }
};

int cmd_help(int argc, char* argv[]) {
	int i = 0;
	printf("Commands:\n");
	for (i = 0; commands[i].name != NULL; i++) {
		printf("  %s \t", commands[i].name);
		if (strlen(commands[i].name) < 5)
            printf(" \t");
		printf("%s\n", commands[i].description);
	}
	printf("\n");
	return 0;
}

int cmd_exit(int argc, char* argv[]) {
    printf("Goodbye\n");
    exit(1);
    return 0;
}

int cmd_echo(int argc, char* argv[]) {
	int i = 0;
	if (argc >= 2) {
		for (i = 1; i < argc; i++) {
			printf("%s ", argv[i]);
		}
		printf("\n");
		return 0;
	}
    
	printf("usage: echo <message>\n");
	return 0;
}

int cmd_hexdump(int argc, char* argv[]) {
	unsigned int len = 0;
	unsigned int addr = 0;
    unsigned char* buf = NULL;
	if (argc != 3) {
		printf("usage: hexdump <address> <length>\n");
		return 0;
	}
    
	addr = (unsigned char*) strtoul(argv[1], 0, 0);
	len = (unsigned int) strtoul(argv[2], 0, 0);
    buf = (unsigned char*) malloc(len);
    
    kern->read(addr, buf, len);
	hexdump(buf, len);
	return 0;
}

int cmd_mw(int argc, char* argv[]) {
	if (argc != 3) {
		printf("usage: mw <address> <value>\n");
		return 0;
	}
    
	unsigned int address = (unsigned int) strtoul(argv[1], 0, 0);
	unsigned int value = (unsigned int) strtoul(argv[2], 0, 0);
    
    kern->set(address, value);
	return 0;
}

int cmd_md(int argc, char* argv[]) {
	if (argc != 2) {
		printf("usage: md <address>\n");
		return 0;
	}
    
    unsigned long address = strtoul(argv[1], 0, 0);
	unsigned int value = kern->get(address);
    
	printf("0x%08x\n", value);
	return value;
}

int cmd_search(int argc, char* argv[]) {
    printf("Not implemented\n");
    return 0;
}

int cmd_string(int argc, char* arg[]) {
    printf("Not implemented\n");
    return 0;
}

int cmd_fuzz(int argc, char* argv[]) {
    printf("Not implemented\n");
    return 0;
}

int cmd_break(int argc, char* argv[]) {
    printf("Not implemented\n");
    return 0;
}

int cmd_base(int argc, char* argv[]) {
    printf("0x%lx\n", kern->base);
    return 0;
}

int cmd_slide(int argc, char* argv[]) {
    printf("0x%lx\n", kern->slide);
    return 0;
}

int cmd_exploit(int argc, char* argv[]) {
    kern->exploit(kern);
    return 0;
}

void lineToArgs(char* line, char*** argv, int* argc) {
    int count = 0;
    char* input = strdup(line);
    char** args = (char**)malloc(sizeof(char*) * 0x100);
    
    char* next = input;
    char* prev = input;
    while(count < 0x100) {
        next = strchr((const char*) next, ' ');
        if(next) {
            next[0] = '\0';
            next++;
            args[count] = prev;
            prev = next;
            
        } else {
            next = prev;
            next = strchr((const char*) next, '\n');
            if(next) {
                next[0] = '\0';
                args[count] = prev;
                
                count++;
                break;
            }
        }
        count++;
    }
    
    *argv = args;
    *argc = count;
}

int kernel_shell() {
    char input[0x200];
    printf("Welcome to Konsole\n");
    printf("Type 'help' for a list of commands\n");
    kern = kernel_open();
    if(kern == NULL) {
        printf("Unable to open kernel reference\n");
        return -1;
    }
    while(1) {
        printf("> ");
        memset(input, '\0', sizeof(input));
        fgets(input, sizeof(input), stdin);
        
        int argc = 0;
        char** argv = NULL;
        lineToArgs(input, &argv, &argc);
        
        if (argc > 0) {
            int i = 0;
            char* cmd = argv[0];
            for (i = 0; commands[i].name != NULL; i++) {
                if(commands[i].name == NULL) {
                    printf("Command %s not found\n", cmd);
                    break;
                    
                } else if (!strcmp(commands[i].name, cmd)) {
                    int ret = commands[i].handler(argc, argv);
                    if(ret) {
                        printf("Error executing command\n");
                    }
                    break;
                }
            }
            
            
        }
    }
    
    return 0;
}