#ifndef MKANCESDIRS_H
#define MKANCESDIRS_H

#include <stddef.h>

struct savewd;
ptrdiff_t mkancesdirs(char*, struct savewd*,
                      int (*)(char*, char*, void*), void*);
#endif
