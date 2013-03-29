#include "file_.h"

#include <fcntl.h>
#include <unistd.h>

ssize_t
pread(int fd, void* buf, ssize_t len, off_t off)
{
    if(lseek(fd, off, SEEK_SET) == (off_t) -1)
        return -1;
    return read(fd, buf, len);
}
