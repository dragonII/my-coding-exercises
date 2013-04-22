/* obstack.h - object stack macros */

/* Summary:

All the apparent functions defined here are macros. The idea
is that you would use these pre-tested macros to solve a
very specific set of problems, and they would run fast.
Caution: no side-effects in arguments please! They may be
evaluated MANY times !

These macros operate a stack of objects. Eack object starts life
small, and may grow to maturity. (Consider building a word syllable
by syllable.) An object can move while it is growing. Once it has
been "finished" it never changes address again. So the "top of the
stack" is typically an immature growing object, while the rest of the
stack is of mature, fixed size and fixed address objects.

These routines grab large chunks of memory, using a function you 
supply, called `obstack_chunk_alloc'. On occasion, they free chunks,
by calling `obstack_chunk_free'. You must define them and declare
them before using any obstack macros.

Each independent stack is represented by a `struct obstack'.
Each of the obstack macros expects a pointer to such a structure 
as the first argument.

One motivation for this package is the problem of growing char strings
in symbol tables. Unless you are "fascist pig with a read-only mind"
--Gosper's immortal quote from HAKMEM item 154, out of context--you
would not like to put any arbitrary upper limit on the length of your
symbols.

In practice this often means you will build many short symbols and a 
few long symbols. At the time you are reading a symbol you don't know
how long it is. One traditional method is to read a symbol into a 
buffer, realloc()ating the buffer every time you try to read a symbol
that is longer than the buffer. This is beaut, but you still will
want to copy the symbol from the buffer to a more permanent
symbol-table entry say about half the time.

With obstacks, you can work differently. Use one obstack for all symbol
names. As you read a symbol, grow the name in the obstack gradually.
When the name is complete, finalize it. Then, if the symbol exists already,
free the newly read name.

The way we do this is to take a large chunk, allocating memory from
low addresses. When you want to build a symbol in the chunk you just
add chars above the current "high water mark" in the chunk. When you
have finished adding chars, because you got to the end of the symbol,
you know how long the chars are, and you can create a new object.
Mostly the chars will not burst over the highest address of the chunk,
because you would typically expect a chunk to be (say) 100 times as
long as an average object.

In case that isn't clear, when we have enough chars to make up
the object, THEY ARE ALREADY CONTIGUOUS IN THE CHUNK (guaranteed)
so we just point to it where it lies. No moving of chars is
needed and this is the second win: potentially long strings need
never be explicitly shuffled. Once an object is formed, it does not
change its address during its lifetime.

When the chars burst over a chunk boundary, we allocate a larger
chunk, and then copy the partly formed object from the end of the old
chunk to the beginning of the new larger chunk. We then carry on
accreting characters to the end of the object as we normally would.

A special macro is provided to add a single char at a time to a
growing object. This allows the use of register variables, which
break the ordinary 'growth' macro.

Summary:
        We allocate large chunks.
        We carve out one object at a time from the current chunk.
        Once carved, an object never moves.
        We are free to append data of any size to the currently
            growing object.
        Exactly one object is growing in an obstack at any one time.
        You can run one obstack per control block.
        You may have as many control blocks as you dare.
        Because of the way we do it, you can `unwind' an obstack
            back to a previous state. (You may remove object much
            as you would with a stack.)
 */

#ifndef _OBSTACK_H
#define _OBSTACK_H

#include <string.h>
#include <stdint.h>
#include <stddef.h>



struct _obstack_chunk               /* lives at front of each chunk */
{
    char *limit;                    /* 1 past end of this chunk */
    struct _obstack_chunk *prev;    /* address of prior chunk or NULL */
    char contents[4];               /* objects begin here */
};


struct obstack      /* control current object in current chunk */
{
    long chunk_size;                /* preferred size to allocate chunks in */
    struct _obstack_chunk *chunk;   /* address of current struct obstack_chunk */
    char *object_base;              /* address of object we are building */
    char *next_free;                /* where to add next char to current object */
    char *chunk_limit;              /* address of char after current chunk */
    union
    {
        PTR_INT_TYPE    tempint;
        void *tempptr;
    } temp;                         /* Temporary for some macros */
    int alignment_mask;             /* Mask of alignment for each object */
    /* These prototypes vary based on `use_extra_arg', and we use
       casts to the prototypeless function type in all assignments,
       but having prototypes here quiets -Wstrict-prototypes */
    struct _obstack_chunk *(*chunkfun) (void *, long);
    void (*freefun) (void *, struct _obstack_chunk *);
    void *extra_arg;                /* first arg for chunk alloc/dealloc funcs */
    unsigned use_extra_arg:1;       /* chunk alloc/dealloc funcs take extra arg */
    unsigned maybe_empty_object:1;  /* There is a possibility that the current
                                       chunk contains a zero-length object. This
                                       prevents freeing the chunk if we allocate
                                       a bigger chunk to replace it */
    unsigned alloc_failed:1;        /* No longer used, as we now call the failed
                                       handler on error, but retained for binary
                                       compatibility */
};


int _obstack_begin(struct obstack *, int, int,
               void *(*)(long), void (*)(void *));



/* To prevent prototype warnings provide complete argument list */
#define obstack_init(h)                                     \
    _obstack_begin ((h), 0, 0,                              \
                    (void *(*) (long)) obstack_chunk_alloc, \
                    (void (*) (void *)) obstack_chunk_free)



#endif
