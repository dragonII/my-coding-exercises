#include <stdio.h>

static ptrx_str_t   *ptrx_sys_errlist;
static ptrx_str_t   ptrx_unknown_error = ptrx_string("Unknown error");
static unsigned int PTRX_SYS_NERR = sys_nerr;

unsigned int
ptrx_strerror_init(void)
{
    char            *msg;
    unsigned char   *p;
    size_t          len;
    ptrx_err_t      err;

    /* 
     * ptrx_strerror() is not ready to work at this stage, therefore,
     * malloc() is used and possible errors are logged using strerror().
     */

    len = PTRX_SYS_NERR * sizeof(ptrx_str_t);

    ptrx_sys_errlist = malloc(len);
    if(ptrx_sys_errlist == NULL)
    {
        goto failed;
    }

    for(err = 0; err < PTRX_SYS_NERR; err++)
    {
        msg = strerror(err);
        len = ptrx_strlen(msg);

        p = malloc(len);
        if(p == NULL)
        {
            goto failed;
        }

        ptrx_memcpy(p, msg, len);
        ptrx_sys_errlist[err].len = len;
        ptrx_sys_errlist[err].data = p;
    }

    return PTRX_OK;

failed:
    err = errno;
    ptrx_log_stderr(0, "malloc(%uz) failed (%d: %s)", len, err, strerror(err));

    return PTRX_ERROR;
}

