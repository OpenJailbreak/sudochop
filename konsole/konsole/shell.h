//
//  kshell.h
//  konsole
//
//  Created by Joshua Hill on 7/22/14.
//
//

#ifndef konsole_kshell_h
#define konsole_kshell_h

typedef int(*cmd_cb_t)(int argc, char* argv[]);

typedef struct {
	char* name;
	cmd_cb_t handler;
	char* description;
} cmd_info_t;

int kernel_shell(void);

#endif
