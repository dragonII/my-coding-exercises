/*
 * Peo Matrix
 */

#include <ptrx_core.h>

static int ptrx_get_options(int argc, char **argv)
{
    unsigned char   *p;
    int             i;

    for(i = 1; i < argc; i++)
    {
        p = (unsigned char*)argv[i];

        if(*p++ != '-')
        {
            ptrx_log_stderr(0, "invalid option: \"%s\"", argv[i]);
            return PTRX_ERROR;
        }

        while(*p)
        {
            switch(*p++)
            {
                case '?':
                    ptrx_show_version = 1;
                    ptrx_show_help = 1;
                    break;
                case 'v':
                    ptrx_show_version = 1;
                    break;

                case 'V':
                    ptrx_show_version = 1;
                    ptrx_show_configure = 1;
                    break;

                /* ... */

                default:
                    ptrx_log_stderr(0, "invalid option: \"%c\"", *(p - 1));
                    return PTRX_ERROR;
            }
        } /* switch(*p++) */

int main(int argc, char **argv)
{
    int                 i;
    ptrx_log_t          *log;
    ptrx_cycle_t        *cycle, init_cycle;
    ptrx_core_conf_t    *ccf;

    if(ptrx_strerror_init() != PTRX_OK)
    {
        return 1;
    }

    if(ptrx_get_options(argc, argv) != PTRX_OK)
    {
        return 1;
    }

//    return 0;
}
