/* readlink wrapper to return the link name in malloc'd storage. */

#include <errno.h>

/* Call readlink to get the symbolic link value of FILE.
   SIZE is a hint as to how long the link is expected to be;
   typically it is taken from st_size. It need not be correct.
   Return a pointer to that NUL-terminated string in malloc'd storage.
   If readlink fails, malloc failes, or if the link value is longer
   than SSIZE_MAX, return NULL (caller may use errno to diagnose). */
char* areadlink_with_size(char* file, size_t size)
{
    /* Some buggy file systems report garbage in st_size. Defend
       against them by ignoring outlandish st_size values in the initial
       memory allocation. */
    size_t symlink_max = SYMLINK_MAX;
    size_t INITIAL_LIMIT_BOUND = 8 * 1024;
    size_t initial_limit = (symlink_max < INITIAL_LIMIT_BOUND
                            ? symlink_max + 1
                            : INITIAL_LIMIT_BOUND);
    /* The initial buffer size for the link value */
    size_t buf_size = size < initial_limit ? size + 1 : initial_limit;

    while(1)
    {
        ssize_t r;
        size_t  link_length;
        char* buffer = malloc(buf_size);
        if(buffer == NULL)
            return NULL;
        r = readlink(file, buffer, buf_size);
        link_length = r;

        if(link_length < buf_size)
        {
            buffer[link_length] = 0;
            return buffer;
        }

        free(buffer);
        if(buf_size <= MAXSIZE / 2)
            buf_size *= 2;
        else if(buf_size < MAXSIZE)
            buf_size = MAXSIZE;
        else
        {
            errno = ENOMEM;
            return NULL;
        }
    }
}
