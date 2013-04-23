#include "obstack.h"

/* Determine default alignment */
union fooround
{
    uintmax_t i;
    long double d;
    void *p;
};
struct fooalign
{
    char c;
    union fooround u;
};


/* If malloc were really smart, it would round addresses to DEFAULT_ALIGNMENT.
   But in fact it might be less smart and round addresses to as much as
   DEFAULT_ROUNDING. So we prepare for it to do that. */
enum
{
    DEFAULT_ALIGNMENT = offsetof(struct fooalign, u),
    DEFAULT_ROUNDING = sizeof(union fooround)
};


/* Define a macro that either calls functions with the traditional malloc/free
   calling interface, or calls functions with the mmalloc/mfree interface
   (that adds an extra first argument), based on the state of use_extra_arg.
   For free, do not use ?:, since some compilers, like the MIPS compilers,
   do not allow (expr) ? void : void. */
#define CALL_CHUNKFUN(h, size)  \
    (((h) -> use_extra_arg)      \
    ? (*(h)->chunkfun) ((h)->extra_arg, (size)) \
    : (*(struct _obstack_chunk *(*) (long)) (h)->chunkfun) ((size)))




/* Initialize an obstack H for use. Specify chunk size SIZE (0 means default).
   Objects start on multiples of ALIGNMENT (0 means use default).
   CHUNKFUN is the function to use to allocate chunks.
   and FREEFUN the function to free them.

   Return nonzero if successful, calls obstack_alloc_failed_handler if
   allocation fails */
int
_obstack_begin(struct obstack *h,
               int size, int alignment,
               void *(*chunkfun)(long),
               void (*freefun)(void *))
{
    register struct _obstack_chunk *chunk;  /* points to new chunk */

    if(alignment == 0)
        alignment = DEFAULT_ALIGNMENT;
    if(size == 0)
        /* Default size is what GNU malloc can fit in a 4096-byte block */
        {
            /* 12 is sizeof (mhead) and 4 is EXTRA from GNU malloc.
               Use the values for range checking, because if range checking is off,
               the extra bytes won't be missed terribly, but if range checking is on
               and we used a larger request, a whole extra 4096 bytes would be
               allocated.

               These number are irrelevant to the new GNU malloc. I suspect it is
               less sensitive to the size of the request */
            int extra = ((((12 + DEFAULT_ROUNDING - 1) & ~(DEFAULT_ROUNDING - 1))
                            + 4 + DEFAULT_ROUNDING - 1)
                            & ~(DEFAULT_ROUNDING - 1));
            size = 4096 - extra;
        }

    h->chunkfun = (struct _obstack_chunk * (*)(void *, long)) chunkfun;
    h->freefun = (void (*) (void *, struct _obstack_chunk *)) freefun;
    h->chunk_size = size;
    h->alignment_mask = alignment - 1;
    h->use_extra_arg = 0;

    chunk = h->chunk = CALL_CHUNKFUN (h, h->chunk_size);
    if(!chunk)
        (*obstack_alloc_failed_handler)();
    h->next_free = h->object_base = __PTR_ALIGN((char *)chunk, chunk->contents,
                                                alignment - 1);
    h->chunk_limit = chunk->limit = (char *)chunk + h->chunk_size;
    chunk->prev = 0;
    /* The initial chunk now contains no empty object */
    h->maybe_empty_object = 0;
    h->alloc_failed = 0;
    return 1;
}


/* Allocate a new current chunk for the obstack *H
   on the assumption that LENGTH bytes need to be added
   to the current object, or a new object of length LENGTH allocated.
   Copies any partial object from the end of the old chunk
   to the beginning of the new one. */
void _obstack_newchunk(struct obstack *h, int length)
{
    register struct _obstack_chunk *old_chunk = h->chunk;
    register struct _obstack_chunk *new_chunk;
    register long new_size;
    register long obj_size = h->next_free - h->object_base;
    register long i;
    long already;
    char *object_base;

    /* Compute size for new chunk */
    new_size = (obj_size + length) + (obj_size >> 3) + h->alignment_mask + 100;
    if(new_size < h->chunk_size)
        new_size = h->chunk_size;

    /* Allocate and initialize the new chunk */
    new_chunk = CALL_CHUNKFUN(h, new_size);
    if(!new_chunk)
        (*obstack_alloc_failed_handler)();

    h->chunk = new_chunk;
    new_chunk->prev = old_chunk;
    new_chunk->limit = h->chunk_limit = (char *)new_chunk + new_size;

    /* compute an aligned object_base in the new chunk */
    object_base =
        __PTR_ALIGN((char *)new_chunk, new_chunk->contents, h->alignment_mask);

    /* move the existing object to the new chunk.
       word at a time is fast and is safe if the object
       is sufficiently aligned */
    if(h->alignment_mask + 1 >= DEFAULT_ALIGNMENT)
    {
        for(i = obj_size / sizeof(COPYING_UNIT) - 1; i >= 0; i--)
            ((COPYING_UNIT *)object_base)[i] =
                    ((COPYING_UNIT *)h->object_base)[i];
        /* we used to copy the odd few remaining bytes as one extra COPYING_UNIT,
           but that can cross a page boundary on a machine
           which does not do strict alignment for COPYING_UNITS. */
        already = obj_size / sizeof(COPYING_UNIT) * sizeof(COPYING_UNIT);
    }
    else
        already = 0;

    /* Copying remaining bytes one by one */
    for(i = already; i < obj_size; i++)
        object_base[i] = h->object_base[i];

    /* If the object just copied was the only data in OLD_CHUNK,
       free that chunk and remove it from the chain.
       But not if that chunk might contain an empty object */
    if(!h->maybe_empty_object
        && (h->object_base ==
            __PTR_ALIGN((char *)old_chunk, old_chunk->contents,
                            h->alignment_mask)))
    {
        new_chunk->prev = old_chunk->prev;
        CALL_FREEFUN(h, old_chunk);
    }

    h->object_base = object_base;
    h->next_free = h->object_base + obj_size;
    /* the new chunk certainly contains no empty object yet */
    h->maybe_empty_object = 0;
}
