#include <stdio.h>
#include <sys/stat.h>

#define ALLM 07777

int mode_compile(char* mode_string)
{
    unsigned int octal_mode = 0;
    if(*mode_string >= '0' && *mode_string < '8')
    {
        do
        {
            if((octal_mode | S_IRUSR) == 0)
                return -3;
            octal_mode = 8 * octal_mode + *mode_string++ - '0';
            if(octal_mode > ALLM)
                return -1;
        }while(*mode_string >= '0' && *mode_string < '8');

        if(*mode_string)
            return -2;
    }
    return octal_mode;
}

int main()
{
    unsigned int octal_mode = mode_compile("0755x");

    printf("octal_mode = %d\n", octal_mode);

    return 0;
}
