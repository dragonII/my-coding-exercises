#ifndef _H_CDF_
#define _H_CDF_

#include <sys/types.h>
#include <stdint.h>

typedef uint64_t cdf_timestamp_t;
typedef int32_t  cdf_secid_t;

#define CDF_LOOP_LIMIT      10000

#define CDF_SECID_NUMM      0
#define CDF_SECID_FREE      -1
#define CDF_SECID_END_OF_CHAIN              -2
#define CDF_SECID_SECTOR_ALLOCATION_TABLE   -3
#define CDF_SECID_MASTER_SECTOR_ALLOCATION_TABLE    -4

#define CDF_TIME_PREC   10000000
#define CDF_BASE_YEAR   1601

typedef struct
{
    int i_fd;
    const unsigned char* i_buf;
    size_t i_len;
} cdf_info_t;

typedef struct
{
    uint64_t    h_magic;
#define CDF_MAGIC   0xE11AB1A1E011CFD0LL
    uint64_t    h_uuid[2];
    uint16_t    h_revision;
    uint16_t    h_version;
    uint16_t    h_byte_order;
    uint16_t    h_sec_size_p2;
    uint16_t    h_short_sec_size_p2;
    uint8_t     h_unused0[10];
    uint32_t    h_num_sectors_in_sat;
    uint32_t    h_secid_first_directory;
    uint8_t     h_unused1[4];
    uint32_t    h_min_size_standard_stream;
    cdf_secid_t h_secid_first_sector_in_short_sat;
    uint32_t    h_num_sectors_in_short_sat;
    cdf_secid_t h_secid_first_sector_in_master_sat;
    uint32_t    h_num_sectors_in_master_sat;
    cdf_secid_t h_master_sat[436 / 4];
} cdf_header_t;


#define CDF_SEC_SIZE(h) ((size_t)(1 << (h)->h_sec_size_p2))
#define CDF_SEC_POS(h, secid) (CDF_SEC_SIZE(h) + (secid) * CDF_SEC_SIZE(h))
#define CDF_SHORT_SEC_SIZE(h)   ((size_t)(1 << (h)->h_short_sec_size_p2))
#define CDF_SHORT_SEC_POS(h, secid) ((secid) * CDF_SHORT_SEC_SIZE(h))


typedef struct
{
    cdf_secid_t* sat_tab;
    size_t       sat_len;
} cdf_sat_t;

typedef uint32_t cdf_dirid_t;

typedef struct
{
    uint16_t    d_name[32];
    uint16_t    d_namelen;
    uint8_t     d_type;
#define CDF_DIR_TYPE_EMPTY          0
#define CDF_DIR_TYPE_USER_STORAGE   1
#define CDF_DIR_TYPE_USER_STREAM    2
#define CDF_DIR_TYPE_LOCKBYTES      3
#define CDF_DIR_TYPE_PROPERTY       4
#define CDF_DIR_TYPE_ROOT_STORAGE   5
    uint8_t     d_color;
#define CDF_DIR_COLOR_READ      0
#define CDF_DIR_COLOR_BLACK     1
    cdf_dirid_t d_left_child;
    cdf_dirid_t d_right_child;
    cdf_dirid_t d_storage;
    uint64_t    d_storage_uuid[2];
    uint32_t    d_flags;
    cdf_timestamp_t d_created;
    cdf_timestamp_t d_modified;
    cdf_secid_t d_stream_first_sector;
    uint32_t    d_size;
    uint32_t    d_unused0;
} cdf_directory_t;

#define CDF_DIRECTORY_SIZE  128

typedef struct
{
    void* sst_tab;
    size_t sst_len;
    size_t sst_dirlen;
} cdf_stream_t;

typedef struct
{
    cdf_directory_t* dir_tab;
    size_t dir_len;
} cdf_dir_t;


int cdf_read_header(const cdf_info_t* info, cdf_header_t* h);
int cdf_read_sat(const cdf_info_t*, cdf_header_t*, cdf_sat_t*);

void cdf_dump_header(const cdf_header_t*);
void cdf_dump_sat(const char*, const cdf_sat_t*, size_t);
int cdf_read_ssat(const cdf_info_t* info, const cdf_header_t* h,
                 const cdf_sat_t* sat, cdf_sat_t* ssat);
void cdf_dump_dir(const cdf_info_t* info, const cdf_header_t* h, 
        const cdf_sat_t* sat, const cdf_sat_t* ssat, 
        const cdf_stream_t* sst, const cdf_dir_t* dir);


ssize_t cdf_read_sector(const cdf_info_t*, void*, size_t, 
                size_t, const cdf_header_t*, cdf_secid_t);
int cdf_read_dir(const cdf_info_t* info, const cdf_header_t* h,
                const cdf_sat_t* sat, cdf_dir_t* dir);
int cdf_read_short_stream(const cdf_info_t* info, const cdf_header_t* h,
         const cdf_sat_t* sat, const cdf_dir_t* dir, cdf_stream_t* scn);

char* cdf_ctime1(const time_t* sec);
char* cdf_ctime(const time_t* sec, char* buf);

#endif
