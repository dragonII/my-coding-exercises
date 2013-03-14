/* pwd - print current directory */

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <error.h>

#include "system.h"
#include "quote.h"
#include "root-dev-ino.h"

#define PROGRAM_NAME "pwd"
