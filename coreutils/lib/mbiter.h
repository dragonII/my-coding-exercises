#ifndef _MBITER_H
#define _MBITER_H

struct mbiter_multi
{
    const char* limit;      /* pointer to end of string */
    bool  in_shift;         /* true if next byte may not be interpreted as ASCII */
    mbstate_t  state;       /* if in_shift: current shift state */
    bool  next_done;        /* true if mbi_avail has already filled the following */
    struct mbchar cur;      /* the current character:
                                    const char* cur.ptr     pointer to current character
                                    The following are only valid after mbi_avail.
                                    size_t cur.bytes        #bytes of current character
                                    bool cur.wc_valid       true if wc is a valid wide character
                                    wchar_t cur.wc          if wc_valid: the current character
                                    */
};


#endif
