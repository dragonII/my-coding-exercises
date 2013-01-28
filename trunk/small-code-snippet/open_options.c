
#define _GNU_SOURCE

#include <stdio.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>


int main()
{
    int fd = -1;
    
    fd = open("/tmp/abcd", (O_RDONLY | O_DIRECTORY | O_NONBLOCK | O_NOFOLLOW));

    if(fd < 0)
    {
        error(0, errno, "cannot open /tmp/abcd");
        return -1;
    }
    close(fd);
    return 0;
}

