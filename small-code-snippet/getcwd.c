#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int main()
{
    char* cwd = getcwd(NULL, 0);
    if(!cwd && errno == ENOMEM)
        printf("No memory\n");
    else
        printf("cwd = %s\n", cwd);

    return 0;
}
