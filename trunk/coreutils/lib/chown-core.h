/* chown-core.h -- types and prototypes shared by chown and chgrp */

#ifndef CHOWN_CORE_H
#define CHOWN_CORE_H

#include <stdbool.h>
#include <sys/types.h>

#include "system.h"

enum Changed_status
{
    CH_NOT_APPLIED = 1,
    CH_SUCCEEDED,
    CH_FAILED,
    CH_NO_CHANGE_REQUESTED
};

enum Verbosity
{
    /* Print a message for each file that is processed */
    V_high,

    /* Print a message for each file whose attributes we change */
    V_changes_only,

    /* Do not be verbose. This is the default */
    V_off
};

struct Chown_option
{
    /* Level of verbosity */
    enum Verbosity verbosity;

    /* If nonzero, change the ownership of directories recursively */
    bool recurse;

    /* Pointer to the device and inode numbers of `/', when --recursive.
       Need not be freed. Otherwise NULL */
    struct dev_ino *root_dev_ino;

    /* This corresponds to the --dereference (opposite of -h) option */
    bool affect_symlink_referent;

    /* If nonzero, force silence (no error message) */
    bool force_silent;

    /* The name of the user to which ownership of the files is being given */
    char* user_name;

    /* The name of the group to which ownership of the files is being given */
    char* group_name;
};


void chopt_init(struct Chown_option* chopt);
void chopt_free(struct Chown_option* chopt ATTRIBUTE_UNUSED);

char* uid_to_name(uid_t uid);
char* gid_to_name(gid_t gid);

bool chown_files(char** files, int bit_flags,
                 uid_t uid, gid_t gid,
                 uid_t required_uid, gid_t required_gid,
                 struct Chown_option* chopt);

#endif
