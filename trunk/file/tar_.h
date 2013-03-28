#ifndef _TAR__H_
#define _TAR__H_

/* Header block on tape.
   
   I'm going to use traditional DP naming conventions here.
   A "block" is a big chunk of stuff that we do I/O on.
   A "record" is a piece of info that we care about.
   Typically many "record"s fit into a "block". */

#define RECORDSIZE  512
#define NAMESIZ     100
#define TUNMLEN     32
#define TGNMLEN     32

union record
{
    unsigned char charptr[RECORDSIZE];
    struct header
    {
        char name[NAMESIZ];
        char mode[8];
        char uid[8];
        char gid[8];
        char size[12];
        char mtime[12];
        char chksum[8];
        char linkflag;
        char linkname[NAMESIZ];
        char magic[8];
        char uname[TUNMLEN];
        char gname[TGNMLEN];
        char devmajor[8];
        char devminor[8];
    } header;
};

/* The magic field is filled with this if uname and gname are valid */
#define TMAGIC      "ustar"     /* 5 chars and a null */
#define GNUTMAGIC   "ustar  "   /* 7 chars and a null */


#endif
