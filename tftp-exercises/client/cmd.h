#ifndef __CMD_H__
#define __CMD_H__

typedef struct CMD
{
    char* cmd_name;         // actual command string
    void  (*cmd_func)();    // pointer to function
} Cmds;

void cmd_ascii();
void cmd_binary();
void cmd_connect();
void cmd_exit();
void cmd_get();
void cmd_help();
void cmd_mode();
void cmd_put();
void cmd_status();
void cmd_verbose();

extern Cmds commands[];
extern int ncmds;

#endif
