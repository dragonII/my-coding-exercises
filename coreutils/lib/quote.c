#include "quote.h"

char* quote_n(int n, char *name)
{
    return name;
}

char *quote(char *name)
{
    return quote_n(0, name);
}
