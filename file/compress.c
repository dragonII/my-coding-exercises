/* compress routines:
    zmagic() - returns 0 if not recognized, uncompresses and prints
                information if recognized
    uncompress(method, old, n, newch) - uncompress old into new,
                using method, return sizof new 
 */

#include "file.h"
#include "magic_.h"

#include <unistd.h>
#include <errno.h>

/* `safe' read for sockets and pipes */
ssize_t 
sread(int fd, void* buf, size_t n, int canbepipe __attribute__((__unused__)))
{
    ssize_t rv;
    size_t rn = n;

    if(fd == STDIN_FILENO)
        goto nocheck;

nocheck:
    do
    {
        switch((rv = read(fd, buf, n)))
        {
            case -1:
                if(errno == EINTR)
                    continue;
                return -1;
            case 0:
                return rn - n;
            default:
                n -= rv;
                buf = ((char*)buf) + rv;
                break;
        }
    } while(n > 0);
    return rn;
}

