#ifndef __ERROR_CLIENT_H__
#define __ERROR_CLIENT_H__

void err_quit(const char* fmt, ...);
void err_sys(const char* fmt, ...);
void err_ret(const char* fmt, ...);
void err_cmd(char* str);

#endif
