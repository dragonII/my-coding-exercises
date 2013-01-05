#ifndef _LONG_OPTIONS_H
#define _LONG_OPTIONS_H

void parse_long_options(int argc, char** argv,
                        char* command_name,
                        char* package,
                        char* version,
                        void(*usage_func)(int),
                        /* char* author1, ...*/ ...);


#endif
