#define _GNU_SOURCE
#include <sys/prctl.h>
#include <sys/capability.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int printmycaps(void *d)
{
    cap_t cap = cap_get_proc();
    printf("Running with uid %d\n", getuid());
    printf("Running with capabilities: %s\n", cap_to_text(cap, NULL));
    cap_free(cap);
    return 0;
}

int exec_with_caps(int newuid, char *capstr, int (*f)(void *data), void *data)
{
    int ret;
    cap_t newcaps;

    ret = prctl(PR_SET_KEEPCAPS, 1);
    if(ret)
    {
        perror("prctl");
        return -1;
    }
    
    ret = setresuid(newuid, newuid, newuid);
    if(ret)
    {
        perror("setresuid");
        return -1;
    }

    newcaps = cap_from_text(capstr);
    ret = cap_set_proc(newcaps);
    if(ret)
    {
        perror("cap_set_proc");
        return -1;
    }

    cap_free(newcaps);
    f(data);

    return 0;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Usage: %s <capability_list>\n",
                argv[0]);
        return 1;
    }
    return exec_with_caps(1000, argv[1], printmycaps, NULL);
}
