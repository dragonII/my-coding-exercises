#ifndef __BASE_H__
#define __BASE_H__

typedef struct
{
    void *ptr;
    size_t used;
    size_t size;
} buffer_plugin;

typedef union
{
#ifdef HAVE_IPV6
    struct sockaddr_in6 ipv6;
#endif
    struct sockaddr_in ipv4;
#ifdef HAVE_SYS_UN_H
    struct sockaddr_un un;
#endif
    struct sockaddr plain;
} sock_addr;

typedef struct server_socket
{
    sock_addr   addr;
    int         fd;
    int         fde_ndx;

    buffer *ssl_pemfile;
    buffer *ssl_ca_file;
    buffer *ssl_cipher_list;
    buffer *ssl_dh_file;
    buffer *ssl_ec_curve;
    unsigned short ssl_use_sslv2;
    unsigned short ssl_use_sslv3;
    unsigned short use_ipv6;
    unsigned short is_ssl;

    buffer *srv_token;
} server_socket;

typedef struct
{
    server_socket **ptr;
    size_t size;
    size_t used;
} server_socket_array;

typedef struct server
{
    server_socket_array srv_sockets;

    /* the errorlog */
    int errorlog_fd;
    enum { ERRORLOG_FILE, ERRORLOG_FD, ERRORLOG_SYSLOG, ERRORLOG_PIPE } errorlog_mode;

    fdevents *ev, *ev_ins;

    buffer_plugin plugins;
    void *plugin_slots;

    /* counters */
    int con_opened;
    int con_read;
    int con_written;
    int con_closed;

    int ssl_is_init;

    int max_fds;        /* max possible fds */
    int cur_fds;        /* currently used fds */
    int want_fds;       /* waiting fds */
    int sockets_disabled;

    size_t max_conns;

    /* buffers */
    buffer *parse_full_path;
    buffer *response_header;
    buffer *response_range;
    buffer *tmp_buf;

    buffer *tmp_chunk_len;
    buffer *emtpy_string;   /* is necessary for cond_match */
    buffer *cond_check_buf;

    /* caches */
    mtime_cache_type mtime_cache[FILE_CACHE_MAX];

    array *split_vals;

    /* timestamps */
    time_t cur_ts;
    time_t last_generated_date_ts;
    time_t last_generated_debug_ts;
    time_t startup_ts;

    char entropy[8];    /* from /dev/[u]random if possible, otherwise rand() */
    char is_read_entropy;   /* whether entropy is from /dev/[u]random */

    buffer *ts_debug_str;
    buffer *ts_date_str;

    /* config-file */
    array *config;
    array *config_touched;

    array *config_context;
    specific_config **config_storage;

    server_config srvconf;

    short int config_deprecated;
    short int config_unsupported;

    connections *conns;
    connections *joblist;
    connections *fdwaitqueue;

    stat_cache *stat_cache;

    /*
     * The status array can carry all the status information you want
     * the key to the array is <module-prefix>.<name>
     * and the values are counters
     *
     * examples:
     *      fastcgi.backends        = 10
     *      fastcgi.active-backends = 6
     *      fastcgi.backend.<key>.load  = 24
     *      fastcgi.backend.<key>...
     *
     *      fastcgi.backend.<key>.disconnects = ...
     */
    array *status;

    fdevent_handler_t event_handler;

    int (* network_backend_write)(struct server *srv, connection *con, int fd, chunkqueue *cq, off_t max_bytes);

    uid_t uid;
    git_t gid;
} server;


#endif
