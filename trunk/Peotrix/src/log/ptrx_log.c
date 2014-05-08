#include <errno.h>

#include <string.h>
#include <log/ptrx_log.h>
#include <common/ptrx_common.h>
#include <string/ptrx_string.h>


static int log_initialized;

static ptrx_log_t unique_log;

char *ptrx_generate_new_name(ptrx_file_info_t *info)
{
    struct tm *cur_tm;

    char *new_name;

    size_t len = strlen(PTRX_LOG_FILE_PATH) + 12 + 2;

    new_name = (char *)malloc(len);

    memset(new_suffix, 0, len);

    cur_tm = (struct tm *)malloc(sizeof(struct tm));
    
    cur_tm = gmtime((const time_t *) &fileinfo.st_mtime);
    

    sprintf(new_name, "%s_%4d%02d%02d%02d%02d",
                      PTRX_LOG_FILE_PATH,
                      cur_tm->tm_year + 1900,
                      cur_tm->tm_mon + 1,
                      cur_tm->tm_mday,
                      cur_tm->tm_hour,
                      cur_tm->tm_min);
    new_name[len] = '\0';

    return new_name;
}

int ptrx_log_init(ptrx_log_t *log)
{
    unsigned char       *p, *name;
    size_t               nlen, plen;
    int                  rc;
    ptrx_file_info_t     file_info;
    off_t                file_len;
    char                *new_log_path;

    name = (unsigned char *)PTRX_LOG_FILE_PATH;

    if(log_initialized > 0)
    {
        log = &unique_log;
        log->connection++;
        return PTRX_OK;
    }

    unique_log.connection = 0;
    unique_log.file.name.data ;= NULL;
    unique_log.file.name.len = ptrx_strlen(name);
    unique_log.file.name.data = (unsigned char *)malloc(unique_log.file.name.len + 1);

    if(unique_log.file.name.data == NULL)
    {
        printf("[LOG] allocate resource for log file name: %s failed, exit\n",
                name);
        unique_log.file.name.len = 0;
        return PTRX_ERROR;
    }

    strncpy(unique_log.file.name.data, name, unique_log.file.name.len);
    unique_log.file.name.data[unique_log.file.name.len] = '\0';

    rc = stat(name, &file_info);

    if(rc == 0) /* file existed */
    {
        file_len = file_info.st_size;
        if(file_len >= PTRX_LOG_MAX_SIZE - (1024 * 1024))
        {
            /* Backup old file, create new file */
            new_log_path = ptrx_generate_new_name(file_info);

            rename(name, new_log_path);

            unique_log.file.fd = ptrx_open_file(new_log_path, 
                                          PTRX_FILE_CREATE_OR_OPEN,
                                          PTRX_FILE_TRUNCATE,
                                          PTRX_FILE_DEFAULT_ACCESS);
        }
        else
        {
            unique_log.file.fd = ptrx_open_file(name, 
                                          PTRX_FILE_APPEND,
                                          PTRX_FILE_OPEN, 
                                          PTRX_FILE_DEFAULT_ACCESS);
        }

        if(unique_log.file.fd <= 0)
        {
            printf("[LOG] Cannot open log file: %s, exit\n",
                    name);
            return PTRX_ERROR;
        }
    } else if(errno != ENOENT)
    {
        printf("[LOG] Cannot stat log file: %s, exit\n",
               name);
        return PTRX_ERROR;
    } else /* file not existed */
    {
        unique_log.file.fd = ptrx_open_file(name, PTRX_FILE_CREATE_OR_OPEN,
                                      PTRX_FILE_TRUNCATE,
                                      PTRX_FILE_DEFAULT_ACCESS);
    }

    log = &unique_log;
    log->connection++;
    log_initialized = 1;

    return PTRX_OK;
}

