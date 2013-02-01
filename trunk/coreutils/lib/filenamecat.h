/* Concatenate two arbitrary file names */

#ifndef FILE_NAME_CONCAT_H
#define FILE_NAME_CONCAT_H

char* file_name_concat(char* dir, char* base,
                        char** base_in_result);
char* mfile_name_concat(char* dir, char* base,
                        char** base_in_result);


#endif
