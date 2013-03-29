#ifndef _H_CDF_
#define _H_CDF_

#include <sys/types.h>
#include <stdint.h>

typedef uint64_t cdf_timestamp_t;
typedef int32_t  cdf_secid_t;

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

#endif
