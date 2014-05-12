/************************************************************************************
 * ** Project    : VSBL
 * ** Filename   : SysSettingGet.cpp
 * ** Creator    : Wang Bin
 * ** Date       : Sep 9, 2009
 * ** Description: Get the item and his value
 * **   
 * ** Copyright (c) 2009 Syslive Co.,Ltd. All Rights Reserved
 * **
 * ** Version History
 * ** ---------------
 * ** Version    : 1.0.0
 * ** Date       : Sep 9, 2009
 * ** Revised by : Wang Bin
 * ** Description: Preliminary release
 * **************************************************************************************/

//#include "SysSetting.h"
#include <string>
#include <iostream>
#include <fstream>


using namespace std;
/* Read a character util ' ','\n' and '\0' encountered, then stop */
void get_arg(char *buf, const char *instr)
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

/* From the configuration file, get the value specified by seciton and item */
void SysSettingGet(string filename, string section, string item, string& val)
{
    fstream file;
    const char *p, *pval, *pbuf;
    int sfind = 0, ifind = 0;
    char line[1024], buf[128];
    section = "[" + section;
    section = section + "]";
    file.open(filename.data(), ifstream::in);
    if(!file) 
      {
        cout << "open fialed !!!" << endl;
        return;
      }
    while(1)
    { 
        /* Read a line string */
        if(file.eof())
            break;
        else
            file.getline(line, sizeof(line), '\n');
        p = line;
        while(isspace(*p)) 
        {
            p++;
        }

        if(*p == '\0' || *p == '#')
        {
            continue;
        }
        get_arg(buf, p);
        pbuf = buf;
        if(strlen(buf) == section.length())
        if(!section.compare(pbuf))
        {
            /* Read the Section */
            sfind++;
            while(1)
            {
                if(file.eof())
                    break;
                else
                    file.getline(line, sizeof(line), '\n');
                p = line;
                while(isspace(*p))
                {
                    p++;
                }
                if(*p == '\0' || *p == '#')
                {
                    continue;
                }
                if(*p == '[')
                {
                    cout << "not find the item under the Section\n" << endl;
                    return;
                }
                get_arg(buf, p);     
                pbuf = buf;
                if(strlen(buf) == item.length())
                if(!item.compare(pbuf))
                {
                /* Read the item option to add the 'VALUE' to val */
                    ifind++;
                    while(isspace(*p))
                        p++;                        
                    while(*pbuf)
                    {
                        p++;
                        pbuf++;
                    }
                    while(isspace(*p))
                    {
                        p++;
                    }
                    if(*p != '=')
                    {
                        cout << " Format does not!! " << endl;
                        return;
                    }
                    else
                    {
                        p++;
                    }
                    pval = p;
                    val.assign(pval);
                    break;
                }
            }	
        }
    }
    if(sfind == 0)
    {
        cout << "Not find the Section\n" << endl;
        return ;    
    }
    else if(ifind == 0)
    {
        cout << "Not find the item under the Section\n" << endl;
        return ;
    }
}

/* From the configuration file, get the value specified by seciton and item */
string SysConfigGet(string filename, string item)
{
    fstream fileS;
    size_t found;
    char line[1024];
    string lineStr;
    string StringValue = "";

    fileS.open(filename.data(), ifstream::in);
    if(!fileS) 
    {
        cout << "open fialed !!!" << endl;
        return "";
    }
    while(1)
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
            //WebService.Endpoint=http://192.168.3.91:8080/SysWebservices/services/IndexWebServices
            found = lineStr.find("=");
            StringValue = lineStr.substr(found + 1);
            break;
        }
    }
    fileS.close();
    return StringValue; 
}

