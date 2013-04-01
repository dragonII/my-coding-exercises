#include "file_.h"
#include "cdf.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <err.h>        /* warn(const char* fmt, ...) */
#include <ctype.h>      /* isprint */

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


char* cdf_ctime1(const time_t* sec)
{
    char* ptr = ctime(sec);
    if(ptr != NULL)
        return ptr;
    ptr = (char*)malloc(48);
    snprintf(ptr, 26, "*Bad* 0x%16.16llx\n", (long long)*sec);
    return ptr;
}


char* cdf_ctime(const time_t* sec, char* buf)
{
    char* ptr = ctime_r(sec, buf);
    if(ptr != NULL)
        return buf;
    snprintf(buf, 26, "*Bad* 0x%16.16llx\n", (long long)*sec);
    return buf;
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


void cdf_unpack_dir(cdf_directory_t* d, char* buf)
{
    size_t len = 0;

    CDF_UNPACKA(d->d_name);
    CDF_UNPACK(d->d_namelen);
    CDF_UNPACK(d->d_type);
    CDF_UNPACK(d->d_color);
    CDF_UNPACK(d->d_left_child);
    CDF_UNPACK(d->d_right_child);
    CDF_UNPACK(d->d_storage);
    CDF_UNPACKA(d->d_storage_uuid);
    CDF_UNPACK(d->d_flags);
    CDF_UNPACK(d->d_created);
    CDF_UNPACK(d->d_modified);
    CDF_UNPACK(d->d_stream_first_sector);
    CDF_UNPACK(d->d_size);
    CDF_UNPACK(d->d_unused0);
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


void cdf_swap_dir(cdf_directory_t* d)
{
    d->d_namelen = CDF_TOLE2(d->d_namelen);
    d->d_left_child = CDF_TOLE4((uint32_t)d->d_left_child);
    d->d_right_child = CDF_TOLE4((uint32_t)d->d_right_child);
    d->d_storage = CDF_TOLE4((uint32_t)d->d_storage);
    d->d_storage_uuid[0] = CDF_TOLE8(d->d_storage_uuid[0]);
    d->d_storage_uuid[1] = CDF_TOLE8(d->d_storage_uuid[1]);
    d->d_flags = CDF_TOLE4(d->d_flags);
    d->d_created = CDF_TOLE8((uint64_t)d->d_created);
    d->d_modified = CDF_TOLE8((uint64_t)d->d_modified);
    d->d_stream_first_sector = CDF_TOLE4((uint32_t)d->d_stream_first_sector);
    d->d_size = CDF_TOLE4(d->d_size);
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


void cdf_dump_header(const cdf_header_t* h)
{
    size_t i;

#define DUMP(a, b)  (void)fprintf(stderr, "%40.40s = " a "\n", # b, h->h_ ## b)
#define DUMP2(a, b) (void)fprintf(stderr, "%40.40s = " a " (" a ")\n", # b, \
                                  h->h_ ## b, 1 << h->h_ ## b)
    DUMP("%d", revision);
    DUMP("%d", version);
    DUMP("0x%x", byte_order);
    DUMP2("%d", sec_size_p2);
    DUMP2("%d", short_sec_size_p2);
    DUMP("%d", num_sectors_in_sat);
    DUMP("%d", secid_first_directory);
    DUMP("%d", min_size_standard_stream);
    DUMP("%d", secid_first_sector_in_short_sat);
    DUMP("%d", num_sectors_in_short_sat);
    DUMP("%d", secid_first_sector_in_master_sat);
    DUMP("%d", num_sectors_in_master_sat);
    for(i = 0; i < __arraycount(h->h_master_sat); i++)
    {
        if(h->h_master_sat[i] == CDF_SECID_FREE)
            break;
        fprintf(stderr, "%35.35s[%.3zu] = %d\n",
                "master_sat", i, h->h_master_sat[i]);
    }
}


void cdf_dump_sat(const char* prefix, const cdf_sat_t* sat, size_t size)
{
    size_t i, j, s = size / sizeof(cdf_secid_t);

    for(i = 0; i < sat->sat_len; i++)
    {
        fprintf(stderr, "%s[%" SIZE_T_FORMAT "u]:\n%.6"
                        SIZE_T_FORMAT "u: ", prefix, i, i * s);
        for(j = 0; j < s; j++)
        {
            fprintf(stderr, "%5d, ",
                    CDF_TOLE4(sat->sat_tab[s * i + j]));
            if((j + 1) % 10 == 0)
                fprintf(stderr, "\n%.6" SIZE_T_FORMAT
                        "u: ", i * s + j + 1);
        }
        fprintf(stderr, "\n");
    }
}


void cdf_dump_dir(const cdf_info_t* info, const cdf_header_t* h, 
        const cdf_sat_t* sat, const cdf_sat_t* ssat, 
        const cdf_stream_t* sst, const cdf_dir_t* dir)
{
    size_t i, j;
    cdf_directory_t* d;
    char name[__arraycount(d->d_name)];
    cdf_stream_t scn;
    struct timespec ts;

    static const char* types[] =
    {
        "emtpy",
        "user storage",
        "user stream",
        "lockbytes",
        "property",
        "root storage"
    };

    for(i = 0; i < dir->dir_len; i++)
    {
        d = &dir->dir_tab[i];
        for(j = 0; j < sizeof(name); j++)
            name[j] = (char)CDF_TOLE2(d->d_name[j]);

        fprintf(stderr, "Directory %" SIZE_T_FORMAT "u: %s\n",
                    i, name);
        if(d->d_type < __arraycount(types))
            fprintf(stderr, "Type: %s\n", types[d->d_type]);
        else
            fprintf(stderr, "Type: %d\n", d->d_type);

        fprintf(stderr, "Color: %s\n",
                    d->d_color ? "black" : "red");
        fprintf(stderr, "Left child: %d\n", d->d_left_child);
        fprintf(stderr, "Right child: %d\n", d->d_right_child);
        fprintf(stderr, "Flags: 0x%x\n", d->d_flags);
        cdf_timestamp_to_timespec(&ts, d->d_created);
        fprintf(stderr, "Created %s", cdf_ctime1(&ts.tv_sec));
        cdf_timestamp_to_timespec(&ts, d->d_modified);
        fprintf(stderr, "Modified %s", cdf_ctime1(&ts.tv_sec));
        fprintf(stderr, "Stream %d\n", d->d_stream_first_sector);
        fprintf(stderr, "Size %d\n", d->d_size);
        switch(d->d_type)
        {
            case CDF_DIR_TYPE_USER_STORAGE:
                fprintf(stderr, "Storage: %d\n", d->d_storage);
                break;
            case CDF_DIR_TYPE_USER_STREAM:
                if(sst == NULL)
                    break;
                if(cdf_read_sector_chain(info, h, sat, ssat, sst,
                     d->d_stream_first_sector, d->d_size, &scn) == -1)
                {
                    warn("Can't read stream for %s at %d len %d",
                        name, d->d_stream_first_sector, d->d_size);
                    break;
                }
                cdf_dump_stream(h, &scn);
                free(scn.sst_tab);
                break;
            default:
                break;
        }
    }
}


/* Read the sector allocation table */
int
cdf_read_sat(const cdf_info_t* info, cdf_header_t* h, cdf_sat_t* sat)
{
    size_t i, j, k;
    size_t ss = CDF_SEC_SIZE(h);
    cdf_secid_t *msa, mid, sec;
    size_t nsatpersec = (ss / sizeof(mid)) - 1;

    for(i = 0; i < __arraycount(h->h_master_sat); i++)
        if(h->h_master_sat[i] == CDF_SECID_FREE)
            break;

#define CDF_SEC_LIMIT (UINT32_MAX / (4 * ss))
    if((nsatpersec > 0 &&
            h->h_num_sectors_in_master_sat > CDF_SEC_LIMIT / nsatpersec)
            || i > CDF_SEC_LIMIT)
    {
        DPRINTF(("Number of sectors in master SAT too big %u %"
                 SIZE_T_FORMAT "u\n", h->h_num_sectors_in_master_sat, i));
        errno = EFTYPE;
        return -1;
    }

    sat->sat_len = h->h_num_sectors_in_master_sat * nsatpersec + i;
    DPRINTF(("sat_len = %" SIZE_T_FORMAT "u ss = %" SIZE_T_FORMAT "u\n",
                sat->sat_len, ss));
    if((sat->sat_tab = CAST(cdf_secid_t*, calloc(sat->sat_len, ss))) == NULL)
        return -1;

    for(i = 0; i < __arraycount(h->h_master_sat); i++)
    {
        if(h->h_master_sat[i] < 0)
            break;
        if(cdf_read_sector(info, sat->sat_tab, ss * i, ss, h,
            h->h_master_sat[i]) != (ssize_t)ss)
        {
            DPRINTF(("Reading error %d", h->h_master_sat[i]));
            goto out1;
        }
    }

    if((msa = CAST(cdf_secid_t*, calloc(1, ss))) == NULL)
        goto out1;

    mid = h->h_secid_first_sector_in_master_sat;
    for(j = 0; j < h->h_num_sectors_in_master_sat; j++)
    {
        if(mid < 0)
            goto out;
        if(j >= CDF_LOOP_LIMIT)
        {
            DPRINTF(("Reading master sector loop limit"));
            errno = EFTYPE;
            goto out2;
        }
        if(cdf_read_sector(info, msa, 0, ss, h, mid) != (ssize_t)ss)
        {
            DPRINTF(("Reading master sector %d", mid));
            goto out2;
        }
        for(k = 0; k < nsatpersec; k++, i++)
        {
            sec = CDF_TOLE4((uint32_t)msa[k]);
            if(sec < 0)
                goto out;
            if(i >= sat->sat_len)
            {
                DPRINTF(("Out of bounds reading MSA %" SIZE_T_FORMAT
                         "u >= %" SIZE_T_FORMAT "u", i, sat->sat_len));
                errno = EFTYPE;
                goto out2;
            }
            if(cdf_read_sector(info, sat->sat_tab, ss * i, ss, h, sec) != (ssize_t)ss)
            {
                DPRINTF(("Reading sector %d", CDF_TOLE4(msa[k])));
                goto out2;
            }
        }
        mid = CDF_TOLE4((uint32_t)msa[nsatpersec]);
    }
out:
    sat->sat_len = i;
    free(msa);
    return 0;
out2:
    free(msa);
out1:
    free(sat->sat_tab);
    return -1;
}


size_t cdf_count_chain(const cdf_sat_t* sat, cdf_secid_t sid, size_t size)
{
    size_t i, j;
    cdf_secid_t maxsector = (cdf_secid_t)(sat->sat_len * size);

    DPRINTF(("Chain:"));
    for(j = i = 0; sid >= 0; i++, j++)
    {
        DPRINTF((" %d", sid));
        if(j >= CDF_LOOP_LIMIT)
        {
            DPRINTF(("Counting chain loop limit"));
            errno = EFTYPE;
            return (size_t)-1;
        }
        if(sid > maxsector)
        {
            DPRINTF(("Sector %d > %d\n", sid, maxsector));
            errno = EFTYPE;
            return (size_t)-1;
        }
        sid = CDF_TOLE4((uint32_t)sat->sat_tab[sid]);
    }
    DPRINTF(("\n"));
    return i;
}


int cdf_read_long_sector_chain(const cdf_info_t* info, const cdf_header_t* h,
        const cdf_sat_t* sat, cdf_secid_t sid, size_t len, cdf_stream_t* scn)
{
    size_t ss = CDF_SEC_SIZE(h), i, j;
    ssize_t nr;
    scn->sst_len = cdf_count_chain(sat, sid, ss);
    scn->sst_dirlen = len;

    if(scn->sst_len == (size_t)-1)
        return -1;

    scn->sst_tab = calloc(scn->sst_len, ss);
    if(scn->sst_tab == NULL)
        return -1;

    for(i = j = 0; sid >= 0; i++, j++)
    {
        if(j >= CDF_LOOP_LIMIT)
        {
            DPRINTF(("Read long sector chain loop limit"));
            errno = EFTYPE;
            goto out;
        }
        if(i >= scn->sst_len)
        {
            DPRINTF(("Out of bounds reading long sector chain "
                    "%" SIZE_T_FORMAT "u > %" SIZE_T_FORMAT "u\n", i,
                    scn->sst_len));
            errno = EFTYPE;
            goto out;
        }
        if((nr = cdf_read_sector(info, scn->sst_tab, i * ss, ss, h,
                                sid)) != (ssize_t)ss)
        {
            if(i == scn->sst_len - 1 && nr > 0)
            {
                /* last sector might be truncated */
                return 0;
            }
            DPRINTF(("Reading long sector chain %d", sid));
            goto out;
        }

        sid = CDF_TOLE4((uint32_t)sat->sat_tab[sid]);
    }
    return 0;

out:
    free(scn->sst_tab);
    return -1;
}


int cdf_read_sector_chain(const cdf_info_t* info, const cdf_header_t* h,
    const cdf_sat_t* sat, const cdf_sat_t* ssat, const cdf_stream_t* sst,
    cdf_secid_t sid, size_t len, cdf_stream_t* scn)
{
    if(len < h->h_min_size_standard_stream && sst->sst_tab != NULL)
        return cdf_read_short_sector_chain(h, ssat, sst, sid, len, scn);
    else
        return cdf_read_long_sector_chain(info, h, sat, sid, len, scn);
}


ssize_t
cdf_read_sector(const cdf_info_t* info, void* buf, size_t offs, 
                size_t len, const cdf_header_t* h, cdf_secid_t id)
{
    size_t ss = CDF_SEC_SIZE(h);
    size_t pos = CDF_SEC_POS(h, id);
    assert(ss == len);
    return cdf_read(info, (off_t)pos, ((char*)buf) + offs, len);
}


int cdf_read_ssat(const cdf_info_t* info, const cdf_header_t* h,
                 const cdf_sat_t* sat, cdf_sat_t* ssat)
{
    size_t i, j;
    size_t ss = CDF_SEC_SIZE(h);
    cdf_secid_t sid = h->h_secid_first_sector_in_short_sat;

    ssat->sat_len = cdf_count_chain(sat, sid, CDF_SEC_SIZE(h));
    if(ssat->sat_len == (size_t)-1)
        return -1;

    ssat->sat_tab = CAST(cdf_secid_t*, calloc(ssat->sat_len, ss));
    if(ssat->sat_tab == NULL)
        return -1;

    for(j = i = 0; sid >= 0; i++, j++)
    {
        if(j >= CDF_LOOP_LIMIT)
        {
            DPRINTF(("Read short sat sector loop limit"));
            errno = EFTYPE;
            goto out;
        }
        if(i >= ssat->sat_len)
        {
            DPRINTF(("Out of bounds reading short sector chain "
                     "%" SIZE_T_FORMAT "u > %" SIZE_T_FORMAT "u\n", i,
                     ssat->sat_len));
            errno = EFTYPE;
            goto out;
        }
        if(cdf_read_sector(info, ssat->sat_tab, i * ss, ss, h, sid) != (ssize_t)ss)
        {
            DPRINTF(("Reading short sat sector %d", sid));
            goto out;
        }
        sid = CDF_TOLE4((uint32_t)sat->sat_tab[sid]);
    }
    return 0;
out:
    free(ssat->sat_tab);
    return -1;
}


int cdf_read_dir(const cdf_info_t* info, const cdf_header_t* h,
                const cdf_sat_t* sat, cdf_dir_t* dir)
{
    size_t i, j;
    size_t ss = CDF_SEC_SIZE(h), ns, nd;
    char* buf;
    cdf_secid_t sid = h->h_secid_first_directory;

    ns = cdf_count_chain(sat, sid, ss);
    if(ns == (size_t)-1)
        return -1;

    nd = ss / CDF_DIRECTORY_SIZE;

    dir->dir_len = ns * nd;
    dir->dir_tab = CAST(cdf_directory_t*,
                        calloc(dir->dir_len, sizeof(dir->dir_tab[0])));
    if(dir->dir_tab == NULL)
        return -1;

    if((buf = CAST(char*, malloc(ss))) == NULL)
    {
        free(dir->dir_tab);
        return -1;
    }

    for(j = i = 0; i < ns; i++, j++)
    {
        if(j >= CDF_LOOP_LIMIT)
        {
            DPRINTF(("Read dir loop limit"));
            errno = EFTYPE;
            goto out;
        }
        if(cdf_read_sector(info, buf, 0, ss, h, sid) != (ssize_t)ss)
        {
            DPRINTF(("Reading directory sector %d", sid));
            goto out;
        }
        for(j = 0; j < nd; j++)
        {
            cdf_unpack_dir(&dir->dir_tab[i * nd + j],
                            &buf[j * CDF_DIRECTORY_SIZE]);
        }
        sid = CDF_TOLE4((uint32_t)sat->sat_tab[sid]);
    }
    if(NEED_SWAP)
        for(i = 0; i < dir->dir_len; i++)
            cdf_swap_dir(&dir->dir_tab[i]);

    free(buf);
    return 0;
out:
    free(dir->dir_tab);
    free(buf);
    return -1;
}


int cdf_read_short_stream(const cdf_info_t* info, const cdf_header_t* h,
         const cdf_sat_t* sat, const cdf_dir_t* dir, cdf_stream_t* scn)
{
    size_t i;
    const cdf_directory_t* d;

    for(i = 0; i < dir->dir_len; i++)
        if(dir->dir_tab[i].d_type == CDF_DIR_TYPE_ROOT_STORAGE)
            break;

    /* if it is not there, just fake it; some docs don't have it */
    if(i == dir->dir_len)
        goto out;
    d = &dir->dir_tab[i];

    /* if it is not there, just fake it; some docs don't have it */
    if(d->d_stream_first_sector < 0)
        goto out;

    return cdf_read_long_sector_chain(info, h, sat,
                    d->d_stream_first_sector, d->d_size, scn);
out:
    scn->sst_tab = NULL;
    scn->sst_len = 0;
    scn->sst_dirlen = 0;
    return 0;
}



void cdf_dump_stream(const cdf_header_t* h, const cdf_stream_t* sst)
{
    ssize_t ss = sst->sst_dirlen < h->h_min_size_standard_stream
                 ? CDF_SHORT_SEC_SIZE(h) : CDF_SEC_SIZE(h);
    cdf_dump(sst->sst_tab, ss * sst->sst_len);
}


int cdf_read_short_sector_chain(const cdf_header_t* h, const cdf_sat_t* ssat,
                                const cdf_stream_t* sst, cdf_secid_t sid,
                                size_t len, cdf_stream_t* scn)
{
    size_t ss = CDF_SHORT_SEC_SIZE(h), i, j;
    scn->sst_len = cdf_count_chain(ssat, sid, CDF_SEC_SIZE(h));
    scn->sst_dirlen = len;

    if(sst->sst_tab == NULL || scn->sst_len == (size_t) -1)
        return -1;

    scn->sst_tab = calloc(scn->sst_len, ss);
    if(scn->sst_tab == NULL)
        return -1;

    for(j = i = 0; sid >= 0; i++, j++)
    {
        if(j >= CDF_LOOP_LIMIT)
        {
            DPRINTF(("Read short sector chain loop limit"));
            errno = EFTYPE;
            goto out;
        }
        if(i >= scn->sst_len)
        {
            DPRINTF(("Out of bounds reading short sector chain "
                     "%" SIZE_T_FORMAT "u > %" SIZE_T_FORMAT "u\n",
                     i, scn->sst_len));
            errno = EFTYPE;
            goto out;
        }
        if(cdf_read_short_sector(sst, scn->sst_tab, i * ss, ss, h,
                sid) != (ssize_t)ss)
        {
            DPRINTF(("Reading short sector chain %d", sid));
            goto out;
        }
        sid = CDF_TOLE4((uint32_t)ssat->sat_tab[sid]);
    }
    return 0;
out:
    free(scn->sst_tab);
    return -1;
}


void cdf_dump(void* v, size_t len)
{
    size_t i, j;
    unsigned char* p = v;
    char abuf[16];
    fprintf(stderr, "%.4x: ", 0);
    for(i = 0, j = 0; i < len; i++, p++)
    {
        fprintf(stderr, "%.2x ", *p);
        abuf[j++] = isprint(*p) ? *p : '.';
        if(j == 16)
        {
            j = 0;
            abuf[15] = '\0';
            fprintf(stderr, "%s\n%.4" SIZE_T_FORMAT "x: ",
                    abuf, i + 1);
        }
    }
    fprintf(stderr, "\n");
}


ssize_t
cdf_read_short_sector(const cdf_stream_t* sst, void* buf, size_t offs,
                      size_t len, const cdf_header_t* h, cdf_secid_t id)
{
    size_t ss = CDF_SHORT_SEC_SIZE(h);
    size_t pos = CDF_SHORT_SEC_POS(h, id);
    assert(ss == len);
    if(pos > CDF_SEC_SIZE(h) * sst->sst_len)
    {
        DPRINTF(("Out of bounds read %" SIZE_T_FORMAT "u > %"
                  SIZE_T_FORMAT "u\n",
                  pos, CDF_SEC_SIZE(h) * sst->sst_len));
        return -1;
    }
    memcpy(((char*)buf) + offs,
            ((const char*)sst->sst_tab) + pos, len);
    return len;
}


static int cdf_namecmp(const char* d, const uint16_t* s, size_t l)
{
    for(; l--; d++, s++)
        if(*d != CDF_TOLE2(*s))
            return (unsigned char)*d - CDF_TOLE2(*s);

    return 0;
}


int 
cdf_read_summary_info(const cdf_info_t* info, const cdf_header_t* h,
                      const cdf_sat_t* sat, const cdf_sat_t* ssat,
                      const cdf_stream_t* sst, const cdf_dir_t* dir,
                      cdf_stream_t* scn)
{
    size_t i;
    const cdf_directory_t* d;
    static const char name[] = "\05SummaryInformation";

    for(i = dir->dir_len; i > 0; i--)
    {
        if(dir->dir_tab[i - 1].d_type == CDF_DIR_TYPE_USER_STREAM &&
            cdf_namecmp(name, dir->dir_tab[i - 1].d_name, sizeof(name)) == 0)
            break;
    }

    if(i == 0)
    {
        DPRINTF(("Cannot find summary information section\n"));
        errno = ESRCH;
        return -1;
    }

    d = &dir->dir_tab[i - 1];
    return cdf_read_sector_chain(info, h, sat, ssat, sst,
                    d->d_stream_first_sector, d->d_size, scn);
}
