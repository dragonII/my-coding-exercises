#include <ptrx_core.h>
#include <ptrx_string.h>

static ptrx_log_t       ptrx_log;
static ptrx_open_file_t ptrx_log_file;

void ptrx_log_stderr(ptrx_err_t err, const char *fmt, ...)
{
    unsigned char *p, *last;
    va_list       args;
    unsigned char errstr[PTRX_MAX_ERROR_STR];

    last = errstr + PTRX_MAX_ERROR_STR;
    p = errstr + 9;

    ptrx_memcpy(errstr, "PeoTrix: ", 9);

    va_start(args, fmt);
    p = ptrx_vslprintf(p, last, fmt, args);
    va_end(args);

    if(err)
    {
        p = ptrx_log_errno(p, last, err);
    }

    if(p > last - PTRX_LINEFEED_SIZE)
    {
        p = last - PTRX_LINEFEED_SIZE;
    }

    ptrx_linefeed(p);

    (void)ptrx_write_console(ptrx_stderr, errstr, p - errstr);
}
ptrx_log_t ptrx_log_init(unsigned char *prefix)
{
    unsigned char *p, *name;
    size_t        nlen, plen;

    ptrx_log.file = &ptrx_log_file;
    ptrx_log.log_level = PTRX_LOG_NOTICE;

    name = (unsigned char *)PTRX_ERROR_LOG_PATH;

    /*
     * we use ptrx_strlen() here because BCC warns about
     * condition is always false and unreachable code
     */

    nlen = ptrx_strlen(name);

    if(nlen == 0)
    {
        ptrx_log_file.fd = ptrx_stderr;
        return &ptrx_log;
    }

    p = NULL;

    if(name[0] != '/')
    {
        if(prefix)
        {
            plen = ptrx_strlen(prefix);
        } else
        {
            prefix = (unsigned char *)PTRX_PREFIX;
            plen = ptrx_strlen(prefix);
        }

        if(plen)
        {
            name = malloc(plen + nlen + 2);
            if(name == NULL)
            {
                return NULL;
            }

            p = ptrx_cpymem(name, prefix, plen);

            if(!ptrx_path_separator(*(p - 1)))
            {
                *p++ = '/';
            }

            ptrx_cpystrn(p, (unsigned char *)PTRX_ERROR_LOG_PATH, nlen + 1);

            p = name;
        }
    }

    ptrx_log_file.fd = ptrx_open_file(name, PTRX_FILE_APPEND,
                                      PTRX_FILE_CREATE_OR_OPEN,
                                      PTRX_FILE_DEFAULT_ACCESS);

    if(ptrx_log_file.fd == PTRX_INVALID_FILE)
    {
        ptrx_log_stderr(ptrx_errno,
                        "[alert] could not open error log file: "
                        ptrx_open_file_n " \"%s\" failed", name);

        ptrx_log_file.fd = ptrx_stderr;
    }
