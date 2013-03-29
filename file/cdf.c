#include "file_.h"
#include "cdf.h"

#include <string.h>
#include <errno.h>

#ifndef EFTYPE
#define EFTYPE EINVAL
#endif

#define DPRINTF(a)  printf a, fflush(stdout)


#define CDF_UNPACK(a)       \
            memcpy(&(a), &buf[len], sizeof(a)), len += sizeof(a)

#define CDF_UNPACKA(a)      \
            memcpy((a), &buf[len], sizeof(a)), len += sizeof(a)


static union
{
    char s[4];
    uint32_t u;
} cdf_bo;


/* swap a short */
static uint16_t
_cdf_tole2(uint16_t sv)
{
    uint16_t rv;
    uint8_t* s = (uint8_t*)(void*)&sv;
    uint8_t* d = (uint8_t*)(void*)&rv;
    d[0] = s[1];
    d[1] = s[0];
    return rv;
}


/* swap an int */
static uint32_t
_cdf_tole4(uint32_t sv)
{
    uint32_t rv;
    uint8_t* s = (uint8_t*)(void*)&sv;
    uint8_t* d = (uint8_t*)(void*)&rv;
    d[0] = s[3];
    d[1] = s[2];
    d[2] = s[1];
    d[3] = s[0];
    return rv;
}


/* swap a quad */
static uint64_t
_cdf_tole8(uint64_t sv)
{
    uint64_t rv;
    uint8_t* s = (uint8_t*)(void*)&sv;
    uint8_t* d = (uint8_t*)(void*)&rv;
    d[0] = s[7];
    d[1] = s[6];
    d[2] = s[5];
    d[3] = s[4];
    d[4] = s[3];
    d[5] = s[2];
    d[6] = s[1];
    d[7] = s[0];
    return rv;
}


#define NEED_SWAP   (cdf_bo.u == (uint32_t)0x01020304)

#define CDF_TOLE8(x)    ((uint64_t)(NEED_SWAP ? _cdf_tole8(x) : (uint64_t)(x)))
#define CDF_TOLE4(x)    ((uint32_t)(NEED_SWAP ? _cdf_tole4(x) : (uint32_t)(x)))
#define CDF_TOLE2(x)    ((uint16_t)(NEED_SWAP ? _cdf_tole2(x) : (uint16_t)(x)))


void cdf_unpack_header(cdf_header_t* h, char* buf)
{
    size_t i;
    size_t len = 0;

    CDF_UNPACK(h->h_magic);
    CDF_UNPACKA(h->h_uuid);
    CDF_UNPACK(h->h_revision);
    CDF_UNPACK(h->h_version);
    CDF_UNPACK(h->h_byte_order);
    CDF_UNPACK(h->h_sec_size_p2);
    CDF_UNPACK(h->h_short_sec_size_p2);
    CDF_UNPACKA(h->h_unused0);
    CDF_UNPACK(h->h_num_sectors_in_sat);
    CDF_UNPACK(h->h_secid_first_directory);
    CDF_UNPACKA(h->h_unused1);
    CDF_UNPACK(h->h_min_size_standard_stream);
    CDF_UNPACK(h->h_secid_first_sector_in_short_sat);
    CDF_UNPACK(h->h_num_sectors_in_short_sat);
    CDF_UNPACK(h->h_secid_first_sector_in_master_sat);
    CDF_UNPACK(h->h_num_sectors_in_master_sat);
    for(i = 0; i < __arraycount(h->h_master_sat); i++)
        CDF_UNPACK(h->h_master_sat[i]);
}


void cdf_swap_header(cdf_header_t* h)
{
    size_t i;

    h->h_magic = CDF_TOLE8(h->h_magic);
    h->h_uuid[0] = CDF_TOLE8(h->h_uuid[0]);
    h->h_uuid[1] = CDF_TOLE8(h->h_uuid[1]);
    h->h_revision = CDF_TOLE2(h->h_revision);
    h->h_version = CDF_TOLE2(h->h_version);
    h->h_byte_order = CDF_TOLE2(h->h_byte_order);
    h->h_sec_size_p2 = CDF_TOLE2(h->h_sec_size_p2);
    h->h_short_sec_size_p2 = CDF_TOLE2(h->h_short_sec_size_p2);
    h->h_num_sectors_in_sat = CDF_TOLE4(h->h_num_sectors_in_sat);
    h->h_secid_first_directory = CDF_TOLE4(h->h_secid_first_directory);
    h->h_min_size_standard_stream = CDF_TOLE4(h->h_min_size_standard_stream);
    h->h_secid_first_sector_in_short_sat = 
            CDF_TOLE4((uint32_t)h->h_secid_first_sector_in_short_sat);
    h->h_num_sectors_in_short_sat = 
            CDF_TOLE4(h->h_num_sectors_in_short_sat);
    h->h_secid_first_sector_in_master_sat =
            CDF_TOLE4((uint32_t)h->h_num_sectors_in_master_sat);
    h->h_num_sectors_in_master_sat =
            CDF_TOLE4(h->h_num_sectors_in_master_sat);
    for(i = 0; i < __arraycount(h->h_master_sat); i++)
        h->h_master_sat[i] = CDF_TOLE4((uint32_t)h->h_master_sat[i]);
}



static ssize_t
cdf_read(const cdf_info_t* info, off_t off, void* buf, size_t len)
{
    size_t siz = (size_t)off  + len;

    if((off_t)(off + len) != (off_t)siz)
    {
        errno = EINVAL;
        return -1;
    }

    if(info->i_buf != NULL && info->i_len >= siz)
    {
        memcpy(buf, &info->i_buf[off], len);
        return (ssize_t)len;
    }

    if(info->i_fd == -1)
        return -1;

    //return -1;

    if(pread(info->i_fd, buf, len, off) != (ssize_t)len)
        return -1;

    return (ssize_t)len;
}

int cdf_read_header(const cdf_info_t* info, cdf_header_t* h)
{
    char buf[512];

    memcpy(cdf_bo.s, "\01\02\03\04", 4);
    if(cdf_read(info, (off_t)0, buf, sizeof(buf)) == -1)
        return -1;

    cdf_unpack_header(h, buf);
    cdf_swap_header(h);
    if(h->h_magic != CDF_MAGIC)
    {
        DPRINTF(("Bad magic 0x%" INT64_T_FORMAT 
                 "x != 0x%" INT64_T_FORMAT "x\n",
                 (unsigned long long)h->h_magic,
                 (unsigned long long)CDF_MAGIC));
        goto out;
    }
    if(h->h_sec_size_p2 > 20)
    {
        DPRINTF(("Bad sector size 0x%u\n", h->h_sec_size_p2));
        goto out;
    }
    if(h->h_short_sec_size_p2 > 20)
    {
        DPRINTF(("Bad short sector size 0x%u\n", h->h_short_sec_size_p2));
        goto out;
    }
    return 0;
out:
    errno = EFTYPE;
    return -1;
}
