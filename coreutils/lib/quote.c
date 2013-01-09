#include "quote.h"

const char* quote_n(int n, const char *name)
{
    return name;
}

const char *quote(const char *name)
{
    return quote_n(0, name);
}
