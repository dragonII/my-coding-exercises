/* compress routines:
    zmagic() - returns 0 if not recognized, uncompresses and prints
                information if recognized
    uncompress(method, old, n, newch) - uncompress old into new,
                using method, return sizof new 
 */

#include "file_.h"
#include "magic_.h"

#include <sys/types.h>
#include <wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <zlib.h>
#include <stdlib.h>


static const struct {
    const char magic[8];
    size_t maglen;
    const char *argv[3];
    int silent;
} compr[] = {
    { "\037\235", 2, { "gzip", "-cdq", NULL }, 1 },     /* compressed */
    /* Uncompress can get stuck; so use gzip first if we have it
    * Idea from Damien Clark, thanks! */
    { "\037\235", 2, { "uncompress", "-c", NULL }, 1 },	/* compressed */
    { "\037\213", 2, { "gzip", "-cdq", NULL }, 1 },     /* gzipped */
    { "\037\236", 2, { "gzip", "-cdq", NULL }, 1 },     /* frozen */
    { "\037\240", 2, { "gzip", "-cdq", NULL }, 1 },     /* SCO LZH */
    /* the standard pack utilities do not accept standard input */
    { "\037\036", 2, { "gzip", "-cdq", NULL }, 0 },     /* packed */
    { "PK\3\4",   4, { "gzip", "-cdq", NULL }, 1 },     /* pkzipped, */
                        /* ...only first file examined */
    { "BZh",      3, { "bzip2", "-cd", NULL }, 1 },     /* bzip2-ed */
    { "LZIP",     4, { "lzip", "-cdq", NULL }, 1 },
    { "\3757zXZ\0",6,{ "xz", "-cd", NULL }, 1 },        /* XZ Utils */
    { "LRZI",     4, { "lrzip", "-dqo-", NULL }, 1 },   /* LRZIP */
};

#define NODATA  ((size_t)~0)


/* `safe' read for sockets and pipes */
ssize_t 
sread(int fd, void* buf, size_t n, int canbepipe __attribute__((__unused__)))
{
    ssize_t rv;
    size_t rn = n;

    if(fd == STDIN_FILENO)
        goto nocheck;

nocheck:
    do
    {
        switch((rv = read(fd, buf, n)))
        {
            case -1:
                if(errno == EINTR)
                    continue;
                return -1;
            case 0:
                return rn - n;
            default:
                n -= rv;
                buf = ((char*)buf) + rv;
                break;
        }
    } while(n > 0);
    return rn;
}


/* `safe' write for sockets and pipes */
static ssize_t
swrite(int fd, const void* buf, size_t n)
{
    ssize_t rv;
    size_t rn = n;

    do
    {
        switch(rv = write(fd, buf, n))
        {
            case -1:
                if(errno == EINTR)
                    continue;
                return -1;
            default:
                n -= rv;
                buf = CAST(const char*, buf) + rv;
                break;
        }
    } while(n > 0);
    return rn;
}

#define FHCRC       (1 << 1)
#define FEXTRA      (1 << 2)
#define FNAME       (1 << 3)
#define FCOMMENT    (1 << 4)


static size_t
uncompressgzipped(struct magic_set* ms, const unsigned char* old,
                unsigned char** newch, size_t n)
{
    unsigned char flg = old[3];
    size_t data_start = 10;
    z_stream z;
    int rc;

    if(flg & FEXTRA)
    {
        if(data_start + 1 >= n)
            return 0;

        data_start += 2 + old[data_start] + old[data_start + 1] * 256;
    }

    if(flg & FNAME)
    {
        while(data_start < n && old[data_start])
            data_start++;
        data_start++;
    }

    if(flg & FCOMMENT)
    {
        while(data_start < n && old[data_start])
            data_start++;
        data_start++;
    }

    if(flg & FHCRC)
        data_start += 2;

    if(data_start >= n)
        return 0;
    if((*newch = CAST(unsigned char*, malloc(HOWMANY + 1))) == NULL)
    {
        return 0;
    }

    /* XXX: const castaway, via strchr */
    z.next_in = (Bytef*)strchr((const char*)old + data_start,
                old[data_start]);
    z.avail_in = CAST(uint32_t, (n - data_start));
    z.next_out = *newch;
    z.avail_out = HOWMANY;
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    /* LINTED bug in header macro */
    rc = inflateInit2(&z, -15);
    if(rc != Z_OK)
    {
        file_error(ms, 0, "zlib: %s", z.msg);
        return 0;
    }

    n = (size_t)z.total_out;
    (void)inflateEnd(&z);

    /* let's keep the nul-ternimate tradition */
    (*newch)[n] = '\0';

    return n;
}


static size_t 
uncompressbuf(struct magic_set* ms, int fd, size_t method,
            const unsigned char* old, unsigned char** newch, size_t n)
{
    int fdin[2], fdout[2];
    ssize_t r;
    pid_t pid;

    if(method == 2)
        return uncompressgzipped(ms, old, newch, n);

    fflush(stdout);
    fflush(stderr);

    if((fd != -1 && pipe(fdin) == -1) || pipe(fdout) == -1)
    {
        file_error(ms, errno, "cannot create pipe");
        return NODATA;
    }

    switch(pid = fork())
    {
        case 0:     /* child */
            close(0);
            if(fd != -1)
            {
                if(dup(fd) == -1)
                    _exit(1);
                lseek(0, (off_t)0, SEEK_SET);
            } else
            {
                if(dup(fdin[0]) == -1)
                    _exit(1);
                close(fdin[0]);
                close(fdin[1]);
            }

            close(1);
            if(dup(fdout[1]) == -1)
                exit(1);
            close(fdout[0]);
            close(fdout[1]);

            //if(compr[method].silent)
            //    close(2);

            (void)execvp(compr[method].argv[0],
                        (char* const *)(intptr_t)compr[method].argv);

            fprintf(stderr, "exec `%s' failed (%s)\n",
                    compr[method].argv[0], strerror(errno));

            exit(1);
        case -1:
            file_error(ms, errno, "could not fork");
            return NODATA;

        default:        /* parent */
            close(fdout[1]);
            if(fd == -1)
            {
                close(fdin[0]);
                /* fork again, to avoid bloking because both
                   pipes filled */
                switch(fork())
                {
                    case 0:     /* child */
                        close(fdout[0]);
                        if(swrite(fdin[1], old, n) != (ssize_t)n)
                        {
                            fprintf(stderr,
                                    "Write failed (%s)\n",
                                    strerror(errno));
                            exit(1);
                        }
                        exit(0);

                    case -1:
                        fprintf(stderr, "Fork failed (%s)\n",
                                strerror(errno));
                        exit(1);

                    default:   /* parent */
                        break;
                }
                close(fdin[1]);
                fdin[1] = -1;
            }

            if((*newch = (unsigned char*)malloc(HOWMANY + 1)) == NULL)
            {
                fprintf(stderr, "Malloc failed (%s)\n", strerror(errno));
                n = 0;
                goto err;
            }

            if((r = sread(fdout[0], *newch, HOWMANY, 0)) <= 0)
            {
                fprintf(stderr, "Read failed (%s)\n", strerror(errno));
                free(*newch);
                n = 0;
                newch[0] = '\0';
                goto err;
            } else
                n = r;

            /* Nul terminate, as every buffer is handled here */
            (*newch)[n] = '\0';
err:
            if(fdin[1] != -1)
                close(fdin[1]);
            close(fdout[0]);

            while(waitpid(pid, NULL, WNOHANG) != -1)
                continue;

            close(fdin[0]);
            return n;
    }
}

static size_t ncompr = sizeof(compr) / sizeof(compr[0]);

int file_zmagic(struct magic_set* ms, int fd, const char* name,
                const unsigned char* buf, size_t nbytes)
{
    unsigned char* newbuf = NULL;
    size_t i, nsz;
    int rv = 0;
    int mime = ms->flags & MAGIC_MIME;

    if((ms->flags & MAGIC_COMPRESS) == 0)
        return 0;

    for(i = 0; i < ncompr; i++)
    {
        if(nbytes < compr[i].maglen)
            continue;
        if(memcmp(buf, compr[i].magic, compr[i].maglen) == 0 &&
             (nsz = uncompressbuf(ms, fd, i, buf, &newbuf, nbytes)) != NODATA)
        {
            ms->flags &= ~MAGIC_COMPRESS;
            rv = -1;
            if(file_buffer(ms, -1, name, newbuf, nsz) == -1)
                goto error;

            if(mime == MAGIC_MIME || mime == 0)
            {
                if(file_printf(ms, mime ?
                                "compressed-encoding=" : " (") == -1)
                    goto error;
            }

            if((mime == 0 || mime & MAGIC_MIME_ENCODING) &&
                file_buffer(ms, -1, NULL, buf, nbytes) == -1)
            {
                goto error;
            }

            if(!mime && file_printf(ms, ")") == -1)
                goto error;

            rv = 1;
            break;
        }
    }
error:
    free(newbuf);
    ms->flags |= MAGIC_COMPRESS;
    return rv;
}

int file_pipe2file(struct magic_set* ms, int fd, 
                   const void* startbuf, size_t nbytes)
{
    char buf[4096];
    ssize_t r;
    int tfd;
    int te;

    strlcpy(buf, "/tmp/file.XXXXXX", sizeof buf);

    tfd = mkstemp(buf);
    te = errno;
    unlink(buf);
    errno = te;

    if(tfd == -1)
    {
        file_error(ms, errno,
            "cannot create temporary file for pipe copy");
        return -1;
    }

    if(swrite(tfd, startbuf, nbytes) != (ssize_t)nbytes)
        r = 1;
    else
    {
        while((r = sread(fd, buf, sizeof(buf), 1)) > 0)
            if(swrite(tfd, buf, (size_t)r) != r)
                break;
    }

    switch(r)
    {
        case -1:
            file_error(ms, errno, "error copying from pipe to temp file");
            return -1;
        case 0:
            break;
        default:
            file_error(ms, errno, "error while writing to temp file");
            return -1;
    }

    /* We duplicate the file descriptor, because fclose on a
       tmpfile will delete the file, but any open descriptors
       can still access the phantom inode */
    if((fd = dup2(tfd, fd)) == -1)
    {
        file_error(ms, errno, "could not dup descriptor from temp file");
        return -1;
    }

    close(tfd);
    if(lseek(fd, (off_t)0, SEEK_SET) == (off_t)-1)
    {
        file_badseek(ms);
        return -1;
    }
    return fd;
}

