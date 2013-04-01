/* softmagic - interpret variable magic from MAGIC */

#include "file_.h"
#include "magic_.h"


/* Go through the whole list, stopping if you find a match. Process all
   the continuations of that match before returning.

   We support multi-level continuations:

    At any time when processing a successful top-level match, there is a
    current continuation level; it represents the level of the last
    successfully matched continuation.

    Continuations above that level are skipped as, if we seen one, it
    means that the continuation that controls them - i.e. the
    lower-level continuation preceding them - failed to match.

    Continuations below that level are precessed as, if we see one,
    it means we've finished processing or skipped higher-level
    continuations under the control of a successfuly or unsuccessfuly
    lower-level continuation, and are now seeing the next lower-level
    continuation and should process it. The current continuation
    level reverts to the level of the one we're seeing.

    Continuations at the current level are processed as, if we seen 
    one, there's no lower-level continuation that may have failed. 

    If a continuation matches, we bump the current continuation level
    so that higher-level continuations are processed. */

static int
match(struct magic_set* ms, struct magic* magic, uint32_t nmagic,
        const unsigned char* s, size_t nbytes, size_t offset, int mode,
        int text, int flip, int* returnval)
{
    uint32_t magindex = 0;
    unsigned int cont_level = 0;
    int need_separator = 0;
    int returnvalv = 0, e;      /* if a match is found it is set to 1 */
    int firstline = 1;          /* a flag to print X\n X\n- X */
    int printed_something = 0;
    int print = (ms->flags & (MAGIC_MIME | MAGIC_APPLE)) == 0;

    if(returnval == NULL)
        returnval = &returnvalv;

    if(file_check_mem(ms, cont_level) == -1)
        return -1;

    for(magindex = 0; magindex < nmagic; magindex++)
    {
        int flush = 0;
        struct magic* m = &magic[magindex];

        if(m->type != FILE_NAME)
        {
            if((IS_STRING(m->type) &&
                ((text && (m->str_flags & (STRING_BINTEST | STRING_TEXTTEST)) == STRING_BINTEST) ||
                (!text && (m->str_flags & (STRING_TEXTTEST | STRING_BINTEST)) == STRING_TEXTTEST))) ||
                (m->flag & mode) != mode)
            {
                /* skip sub-tests */
                while(magindex + 1 < nmagic &&
                         magic[magindex + 1].cont_level != 0 && ++magindex)
                    continue;
                continue;   /* skip to next top-level test */
            }
        }

        ms->offset = m->offset;
        ms->line = m->lineno;

        /* if main entry matches, print it ...*/
        switch(mget(ms, s, m, nbytes, offset, cont_level, mode, text,
                flip, returnval))
        {
            case -1:
                return -1;
            case 0:
                flush = m->reln != '!';
                break;
            default:
                if(m->type == FILE_INDIRECT)
                    *returnval = 1;

                switch(magiccheck(ms, m))
                {
                    case -1:
                        return -1;
                    case 0:
                        flush++;
                        break;
                    default:
                        flush = 0;
                        break;
                }
                break;
        }
        if(flush)
        {
            /* main entry didn't match,
               flush its continuations */
            while(magindex < nmagic - 1 &&
                magic[magindex + 1].cont_level != 0)
            {
                magindex++;
            }
            continue;
        }

        if((e = handle_annotation(ms, m)) != 0)
        {
            *returnval = 1;
            return e;
        }

        /* If we are going to print something, we'll need to print
           a blank before we print something else */
        if(*m->desc)
        {
            need_separator = 1;
            printed_something = 1;
            if(print_sep(ms, firstline) == -1)
                return -1;
        }

        if(print && mprint(ms, m) == -1)
            return -1;

        ms->c.li[cont_level].off = moffset(ms, m);

        /* and any continuations that match */
        if(file_check_mem(ms, ++cont_level) == -1)
            return -1;

        while(magic[magindex + 1].cont_level != 0 &&
                ++magindex < nmagic)
        {
            m = &magic[magindex];
            ms->line = m->lineno;   /* for messages */

            if(cont_level < m->cont_level)
                continue;
            if(cont_level > m->cont_level)
            {
                /* We're at the end of the level
                   "cont_level" continuations */
                cont_level = m->cont_level;
            }
            ms->offset = m->offset;
            if(m->flag & OFFADD)
            {
                ms->offset += ms->c.li[cont_level - 1].off;
            }
#ifdef ENABLE_CONDITIONALS
            if(m->cond == COND_ELSE || 
                m->cond == COND_ELIF)
            {
                if(ms->c.li[cont_level].last_match == 1)
                    continue;
            }
#endif
            switch(mget(ms, s, m, nbytes, offset, cont_level, mode,
                        text, flip, returnval))
            {
                case -1:
                    return -1;
                case 0:
                    if(m->reln != '!')
                        continue;
                    flush = 1;
                    break;
                default:
                    if(m->type == FILE_INDIRECT)
                        *returnval = 1;
                    flush = 0;
                    break;
            }

            switch(flush ? 1 : magiccheck(ms, m))
            {
                case -1:
                    return -1;
                case 0:
#ifdef ENABLE_CONDITIONALS
                    ms->c.li[cont_level].last_match = 0;
#endif
                    break;
                default:
#ifdef ENABLE_CONDITIONALS
                    ms->c.li[cont_level].last_match = 1;
#endif
                    if(m->type != FILE_DEFAULT)
                        ms->c.li[cont_level].got_match = 1;
                    else if(ms->c.li[cont_level].got_match)
                    {
                        ms->c.li[cont_level].got_match = 0;
                        break;
                    }
                    if((e = handle_annotation(ms, m)) != 0)
                    {
                        *returnval = 1;
                        return e;
                    }
                    /* if we are going to print something,
                       make sure that we have a separator first */
                    if(*m->desc)
                    {
                        if(!printed_something)
                        {
                            printed_something = 1;
                            if(print_sep(ms, firstline) == -1)
                                return -1;
                        }
                    }

                    /* This continuation matched. Print
                       its message, with a blank before it
                       if the previous item printed and
                       this item isn't empty */
                    /* space if previous printed */
                    if(need_separator
                        && ((m->flag & NOSPACE) == 0)
                        && *m->desc)
                    {
                        if(print && file_printf(ms, " ") == -1)
                            return -1;
                        need_separator = 0;
                    }
                    if(print && mprint(ms, m) == -1)
                        return -1;

                    ms->c.li[cont_level].off = moffset(ms, m);

                    if(*m->desc)
                        need_separator = 1;

                    /* If we see any continuations
                       at a higher level, process them */
                    if(file_check_mem(ms, ++cont_level) == -1)
                        return -1;
                    break;
            }
        }
        if(printed_something)
        {
            firstline = 0;
            if(print)
                *returnval = 1;
        }
        if((ms->flags & MAGIC_CONTINUE) == 0 && printed_something)
        {
            return *returnval;  /* don't keep searching */
        }
    }
    return *returnval;  /* This is hit if -k is set or there is no match */
}


/* softmagic - lookup one file in parsed, in-memory copy of database
   Passed the name and FILE* of one file to be typed */

int file_softmagic(struct magic_set* ms, const unsigned char* buf,
                    size_t nbytes, int mode, int text)
{
    struct mlist* ml;
    int rv;
    for(ml = ms->mlist[0]->next; ml != ms->mlist[0]; ml = ml->next)
    {
        if((rv = match(ms, ml->magic, ml->nmagic, buf, nbytes, 0, mode,
                        text, 0, NULL)) != 0)
            return rv;
    }
    return 0;
}
