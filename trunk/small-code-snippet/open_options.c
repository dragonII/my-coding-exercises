
#define _GNU_SOURCE

//#include <stdio.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <dirent.h>
//#include <error.h>
//#include <errno.h>
//#include <unistd.h>
//#include <stdbool.h>


#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <error.h>
 
#include <locale.h>


static inline bool
is_empty_dir(int fd_cwd, char* dir)
{
    DIR* dirp;
    struct dirent* dp; 
    int saved_errno;
    int fd = openat(fd_cwd, dir,
                        (O_RDONLY | O_DIRECTORY
                         | O_NOCTTY | O_NOFOLLOW | O_NONBLOCK));
    
    if(fd < 0)
        return false;
    
    dirp = fdopendir(fd);
    if(dirp == NULL)
    {   
        close(fd);
        return false;
    }   
    
    errno = 0;
    //dp = readdir_ignoring_dot_and_dotdot(dirp);
    saved_errno = errno;
    closedir(dirp);
    if(dp != NULL)
        return false;
    return saved_errno == 0 ? true : false;
}



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

