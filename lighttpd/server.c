#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <locale.h>

#include <stdio.h>
#include <getopt.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/prctl.h>

static int l_issetugid(void) {
    return (geteuid() != getuid() || getegid() != getgid());
}

#define issetugid l_issetugid

static volatile sig_atomic_t srv_shutdown = 0;
static volatile sig_atomic_t graceful_shutdown = 0;
static volatile sig_atomic_t handle_sig_alarm = 1;
static volatile sig_atomic_t handle_sig_hup = 0;
static volatile sig_atomic_t forwarded_sig_hup = 0;


int main(int argc, char **argv)
{
    server *srv = NULL;
