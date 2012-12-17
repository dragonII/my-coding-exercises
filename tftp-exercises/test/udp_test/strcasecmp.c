#include <stdio.h>
#include <string.h>

int main()
{
    char* s1 = "quit";
    char* s2 = "QUIT";

    int z = strcasecmp(s1, s2);

    printf("z = %d\n", z);

    return 0;
}
