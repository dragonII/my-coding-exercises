#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main1()
{
    char* groups = "a,b.c,d.e,f.g,h";
    char* string;
    char* tmp;

    //for(string = strdup(groups); ; tmp = NULL)
    //{
    //    tmp = strtok(string, ",");
    //    if(tmp != NULL)
    //        printf("%s\n", tmp);
    //}

    string = strdup(groups);

    tmp = strtok(string, ",.");
    printf("1%s\n", tmp);

    tmp = strtok(NULL, ",.");
    printf("2%s\n", tmp);

    tmp = strtok(NULL, ",.");
    printf("3%s\n", tmp);

    tmp = strtok(NULL, ",.");
    printf("4%s\n", tmp);

    tmp = strtok(NULL, ",.");
    printf("5%s\n", tmp);



    if(string)
        free(string);

    return 0;
}


int main(int argc, char** argv)
{
    char *str1, *str2, *token, *subtoken;
    int j;

    if(argc != 4)
    {
        fprintf(stderr, "Usage: %s string delim subtoken\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for(j = 1, str1 = argv[1]; ; j++, str1 = NULL)
    {
        token = strtok(str1, argv[2]);
        if(token == NULL)
            break;
        printf("%d: %s\n", j, token);

        for(str2 = token; ; str2 = NULL)
        {
            subtoken = strtok(str2, argv[3]);
            if(subtoken == NULL)
                break;
            printf("  --> %s\n", subtoken);
        }
    }
    exit(EXIT_SUCCESS);
}
