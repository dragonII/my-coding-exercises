#ifndef _TRIM_H
#define _TRIM_H

#define TRIM_TRAILING 0
#define TRIM_LEADING  1
#define TRIM_BOTH     2

/* Removes trailing and leading whitespaces. */
#define trim(s) trim2((s), TRIM_BOTH)

/* Removes trailing whitespaces */
#define trim_trailing(s) trim2((s), TRIM_TRAILING)

/* Removes leading whitespaces */
#define trim_leading(s) trim2((s), TRIM_LEADING)

char* trim2(const char*, int);

#endif
