#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <file/ptrx_file.h>
#include <log/ptrx_log.h>

/* Read a character util ' ','\n' and '\0' encountered, then stop */
static void get_arg(char *buf, char *instr)
{
   const char *p = instr;
   int cont = 0;
    while(*p == '=')
    {
        p++;
        cont = 1;
    }
    while(isspace(*p))
    {
        p++;
    }
    if(cont == 1)
        while(*p && *p != '\n')
        {
            *buf = *p;
            p++;
            buf++;
        }
    else
        while(!isspace(*p) && *p && *p != '=')
        {
            *buf = *p;
            p++;
            buf++;
        }

        *buf = '\0';
}

/* TODO: to be finished */
/* From the configuration file, get the value specified by seciton and item */
char *ptrx_config_get(char *filename, char *item, ptrx_log_t *log)
{
    char *name;
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t readlen;


    if(filename == NULL)
    {
        name = (char *)PTRX_CONFIG_FILE;
    } else
    {
        name = filename;
    }

    fd = ptrx_open_file(name,
                        PTRX_FILE_RDONLY,
                        PTRX_FILE_OPEN,
                        PTRX_FILE_DEFAULT_ACCESS);
    fp = fopen(name, "r");
    if(fp == NULL)
    {
        ptrx_log_stderr(log, PTRX_LOG_ERR, 
                        0, "cannot open file: %s", name);
        return NULL;
    }


    fstream fileS;
    size_t found;
    char line[1024];
    string lineStr;
    string StringValue = "";

    memset(line, 0, 1024);
    //while(1)
    while((readlen = getline(&line, &len, fp)) != -1)
    { 
        /* Read a line string */
        if(fileS.eof())
            break;
        else
            fileS.getline(line, sizeof(line), '\n');
        lineStr = line;
        found = lineStr.find("#");
        if(found != string::npos)
        {
            //cout << "# found, continue" << endl;
            continue;
        }
        found = lineStr.find(item);
        if(found == string::npos)
            continue;
        else
        {
            found = lineStr.find("=");
            StringValue = lineStr.substr(found + 1);
            break;
        }
    }
    fileS.close();
    return StringValue; 
}

