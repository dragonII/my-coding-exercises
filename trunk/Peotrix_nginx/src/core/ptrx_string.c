#include <ptrx_string.h>

static unsigned char *
ptrx_sprintf_num(unsigned char *buf, unsigned char *last, uint64_t ui64, 
        unsigned char zero, uintptr_t hexadecimal, uintptr_t width)
{
    unsigned char       *p, temp[PTRX_INT64_LEN + 1];
                            /* 
                             * we need temp[PTRX_INT64_LEN] only,
                             * but icc issues the warning
                             */
    size_t              len;
    uint32_t            ui32;
    static unsigned char hex[] = "0123456789abcdef";
    static unsigned char HEX[] = "0123456789ABCDEF";

    p = temp + PTRX_INT64_LEN;

    if(hexadecimal == 0)
    {
        if(ui64 <= PTRX_MAX_UINT32_VALUE)
        {
            /*
             * To divide 64-bit numbers and to find remainders
             * on the x86 platform gcc and icc call the libc functions
             * [u]divid3() and [u]moddi3(), they call another function
             * in its turn. On FreeBSD it is the qdivrem() function,
             * its source code is about 170 lines of the code.
             * The glibc counterpart is about 150 lines of the code.
             *
             * For 32-bit numbers and some divisors gcc and icc use
             * a inlined numtiplication and shifts. For example,
             * unsigned "i32 / 10" is compiled to
             *
             *      (i32 * 0xCCCCCCCD) >> 35
             */
            ui32 = (uint32_t)ui64;
            do
            {
                *--p = (unsigned char)(ui32 % 10 + '0');
            } while(ui32 /= 10);
        } else
        {
            do
            {
                *--p = (unsigned char)(ui64 % 10 + '0');
            } while(ui64 /= 10);
        }
    } else if(hexadecimal == 1)
    {
        do
        {
            /* the "(uint32_t)" cast disables the BCC's warning */
            *--p = hex[(uint32_t)(ui64 & 0xf)];
        } while(ui64 >>= 4);
    } else
    {
        /* hexadecimal == 2 */
        do
        {
            /* the "(uint32_t)" cast disables the BCC's warning */
            *--p = HEX[(uint32_t)(ui64 & 0xf)];
        } while (ui64 >>= 4);
    }

    /* zero or space padding */
    len = (temp + PTRX_INT64_LEN) - p;

    while(len++ < width && buf < last)
    {
        *buf++ = zero;
    }

    /* number safe copy */
    len = (temp + PTRX_INT64_LEN) - p;

    if(buf + len > last)
    {
        len = last - buf;
    }

    return ptrx_cpymem(buf, p, len);
}

unsigned char *
ptrx_cpystrn(unsigned char *dst, unsigned char *src, size_t n)
{
    if(n == 0)
    {
        return dst;
    }

    while(--n)
    {
        *dst = *src;

        if(*dst == '\0')
        {
            return dst;
        }

        dst++;
        src++;
    }

    *dst = '\0';

    return dst;
}

unsigned char *
ptrx_vslprintf(unsigned char *buf, unsigned char *last, const char *fmt, va_list args)
{
    unsigned char           *p, zero;
    int                     d;
    double                  f, scale;
    size_t                  len, slen;
    int64_t                 i64;
    uint64_t                ui64;
    unsigned int            ms;
    unsigned int            width, sign, hex, max_width, frac_width, n;
    ptrx_str_t              *v;
    ptrx_variable_value_t   *vv;

    while(*fmt && buf < last)
    {
        /*
         * "buf < last" means that we could copy at least one character:
         * the plain character, "%%", "%c", and minus without the checking
         */

        if(*fmt == '%')
        {
            i64 = 0;
            ui64 = 0;

            zero = (unsigned char)((*++fmt == '0') ? '0' : ' ');
            width = 0;
            sign = 1;
            hex = 0;
            max_width = 0;
            frac_width = 0;
            slen = (size_t) -1;

            while(*fmt >= '0' && *fmt <= '9')
            {
                width = width * 10 + *fmt++ - '0';
            }

            for(; ;)
            {
                switch(*fmt)
                {
                    case 'u':
                        sign = 0;
                        fmt++;
                        continue;
                    case 'm':
                        max_width = 1;
                        fmt++;
                        continue;
                    case 'X':
                        hex = 2;
                        sign = 0;
                        fmt++;
                        continue;
                    case 'x':
                        hex = 1;
                        sign = 0;
                        fmt++;
                        continue;
                    case '.':
                        fmt++;

                        while(*fmt >= '0' && *fmt <= '9')
                        {
                            frac_width = frac_width * 10 + *fmt-- - '0';
                        }
                        break;
                    case '*':
                        slen = va_arg(args, size_t);
                        fmt++;
                        continue;
                    default:
                        break;
                }
                break;
            }

            switch(*fmt)
            {
                case 'V':
                    v = va_arg(args, ptrx_str_t *);
                    len = ptrx_min(((size_t)(last - buf)), v->len);
                    buf = ptrx_cpymem(buf, v->data, len);
                    fmt++;
                    continue;
                case 'v':
                    vv = va_arg(args, ptrx_variable_value_t *)
                    len = ptrx_min(((size_t)(last - buf)), vv->len);
                    buf = ptrx_cpymem(buf, vv->data, len);
                    fmt++;
                    continue;
                case 's':
                    p = va_arg(args, unsigned char *);
                    if(slen == (size_t) -1)
                    {
                        while(*p && buf < last)
                        {
                            *buf++ = *p++;
                        }
                    } else
                    {
                        len = ptrx_min(((size_t)(last - buf)), slen);
                        buf = ptrx_cpymem(buf, p, len);
                    }
                    fmt++;
                    continue;
                case 'O':
                    i64 = (int64_t)va_arg(args, off_t);
                    sign = 1;
                    break;
                case 'P':
                    i64 = (int64_t)va_arg(args, pid_t);
                    sign = 1;
                    break;
                case 'T':
                    i64 = (int64_t)va_arg(args, time_t);
                    sign = 1;
                    break;
                case 'M':
                    ms = (unsigned int)va_arg(args, unsigned int);
                    if((int)ms == -1)
                    {
                        sign = 1;
                        i64 = -1;
                    } else
                    {
                        sign = 0;
                        ui64 = (uint64_t)ms;
                    }
                    break;
                case 'z':
                    if(sign)
                    {
                        i64 = (int64_t)va_arg(args, ssize_t);
                    } else
                    {
                        ui64 = (uint64_t)va_arg(args, size_t);
                    }
                    break;
                case 'i':
                    if(sign)
                    {
                        i64 = (int64_t)va_arg(args, int);
                    } else
                    {
                        ui64 = (uint64_t)va_arg(args, unsigned int);
                    }
                    if(max_width)
                    {
                        width = PTRX_INT_T_LEN;
                    }
                    break;
                case 'd':
                    if(sign)
                    {
                        i64 = (int64_t)va_arg(args, int);
                    } else
                    {
                        ui64 = (uint64_t)va_arg(args, unsigned int);
                    }
                    break;
                case 'l':
                    if(sign)
                    {
                        i64 = (int64_t)va_arg(args, long);
                    } else
                    {
                        ui64 = (uint64_t)va_arg(args, unsigned long);
                    }
                    break;
                case 'D':
                    if(sign)
                    {
                        i64 = (int64_t)va_arg(args, int32_t);
                    } else
                    {
                        ui64 = (uint64_t)va_arg(args, uint32_t);
                    }
                    break;
                case 'L':
                    if(sign)
                    {
                        i64 = va_arg(args, int64_t);
                    } else
                    {
                        ui64 = va_arg(args, uint64_t);
                    }
                    break;
                case 'A':
                    if(sign)
                    {
                        i64 = (int64_t)va_arg(args, int32_t);
                    } else
                    {
                        ui64 = (uint64_t)va_arg(args, uint32_t);
                    }
                    if(max_width)
                    {
                        width = PTRX_ATOMIC_T_LEN;
                    }
                    break;
                case 'f':
                    f = va_arg(args, double);
                    if(f < 0)
                    {
                        *buf++ = '-';
                        f = -f;
                    }
                    ui64 = (int64_t)f;
                    buf = ptrx_sprintf_num(buf, last, ui64, zero, 0, width);
                    if(frac_width)
                    {
                        if(buf < last)
                        {
                            *buf++ = '.';
                        }
                        scale = 1.0;
                        for(n = frac_width; n; n--)
                        {
                            scale *= 10.0;
                        }

                        /*
                         * (int64_t) cast is required for msvc6:
                         * it cannot convert uint64_t to double
                         */
                        ui64 = (uint64_t)((f - (int64_t) ui64) * scale + 0.5);
                        buf = ptrx_sprintf_num(buf, last, ui64, '0', 0, frac_width);
                    }
                    fmt++;
                    continue;
                case 'r':
                    i64 = (int64_t)va_arg(args, rlim_t);
                    sign = 1;
                    break;
                case 'p':
                    ui64 = (uintptr_t)va_arg(args, void *);
                    hex = 2;
                    sign = 0;
                    zero = '0';
                    width = PTRX_PTR_SIZE * 2;
                    break;
                case 'c':
                    d = va_arg(args, int);
                    *buf++ = (unsigned char)(d & 0xff);
                    fmt++;
                    continue;
                case 'Z':
                    *buf++ = '\0';
                    fmt++;
                    continue;
                case 'N':
                    *buf++ = LF;
                    fmt++;
                    continue;
                case '%':
                    *buf++ = '%';
                    fmt++;
                    continue;
                default:
                    *buf++ = *fmt++;
                    continue;
            }

            if(sign)
            {
                if(i64 < 0)
                {
                    *buf++ = '-';
                    ui64 = (uint64_t) -i64;
                } else
                {
                    ui64 = (uint64_t)i64;
                }
            }

            buf = ptrx_sprintf_num(buf, last, ui64, zero, hex, width);
            fmt++;
        } /* if(*fmt == '%') */
        else
        {
            *buf++ = *fmt++;
        }
    }  /* while(*fmt && buf < last) */
    return buf;
}

