/* root-dev-ino.c -- get the device and inode numbers for `/'. */

#include "root-dev-ino.h"
#include "dev-ino.h"
#include <stdlib.h>

/* Call lstat to get the device and inode numbers for `/'.
   Upon failure, return NULL. Otherwise, set the numbers of
   *ROOT_D_I accordingly and return ROOT_D_I. */
struct dev_ino* 
get_root_dev_ino(struct dev_ino* root_d_i)
{
    struct stat statbuf;
    if(lstat("/", &statbuf))
        return NULL;
    root_d_i->st_ino = statbuf.st_ino;
    root_d_i->st_dev = statbuf.st_dev;
    return root_d_i;
}
