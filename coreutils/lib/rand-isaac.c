/* Bob Jenkins's cryptographic random number generator, ISAAC */

#include "rand-isaac.h"

#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "xtime.h"
#include "gethrxtime.h"


/* this index operation is more effiecient on many processors */
#define ind(mm, x) \
    (* (uint32_t *) ((char *) (mm)   \
                    + ((x) & (ISAAC_WORDS - 1) * sizeof (uint32_t))))

/* The central step. This uses two temporaries, x and y. mm is the
   whole state array, while m is a pointer to the current word. off is
   the offset from m to the word ISAAC_WORDS/2 words away in the mm aray,
   i.e. +/- ISAAC_WORDS/2 */
#define isaac_step(mix, a, b, mm, m, off, r) \
(   \
    a = ((a) ^ (mix)) + (m)[off],   \
    x = *(m),   \
    *(m) = y = ind(mm, x) + (a) + (b),  \
    *(r) = b = ind(mm, (y) >> ISAAC_LOG) + x    \
)


/* Use and update *S to generate random data to fill R */
void isaac_refill(struct isaac_state* s, uint32_t r[ISAAC_WORDS])
{
    uint32_t a, b;          /* Caches of a and b */
    uint32_t x, y;          /* Temps needed by isaac_step macro */
    uint32_t *m = s->mm;    /* Pointer into state array */

    a = s->a;
    b = s->b + (++s->c);

    do
    {
        isaac_step(a << 13, a, b, s->mm, m, ISAAC_WORDS / 2, r);
        isaac_step(a >>  6, a, b, s->mm, m + 1, ISAAC_WORDS / 2, r + 1);
        isaac_step(a <<  2, a, b, s->mm, m + 2, ISAAC_WORDS / 2, r + 2);
        isaac_step(a >> 16, a, b, s->mm, m + 3, ISAAC_WORDS / 2, r + 3);
        r += 4;
    }

    while((m += 4) < s->mm + ISAAC_WORDS / 2);
    do
    {
        isaac_step(a << 13, a, b, s->mm, m, -ISAAC_WORDS / 2, r);
        isaac_step(a >>  6, a, b, s->mm, m + 1, -ISAAC_WORDS / 2, r + 1);
        isaac_step(a <<  2, a, b, s->mm, m + 2, -ISAAC_WORDS / 2, r + 2);
        isaac_step(a >> 16, a, b, s->mm, m + 3, -ISAAC_WORDS / 2, r + 3);
        r += 4;
    }
    while((m += 4) < s->mm + ISAAC_WORDS);
    s->a = a;
    s->b = b;
}



/* The basic seed-scrambling step for initialization, based on Bob
   Jenkins's 256-bit hash */
#define mix(a,b,c,d,e,f,g,h) \
   (        a ^= b << 11, d += a,   \
    b += c, b ^= c >>  2, e += b,   \
    c += d, c ^= d >>  8, f += c,   \
    d += e, d ^= e >> 16, g += d,   \
    e += f, e ^= f >> 10, h += e,   \
    f += g, f ^= g >>  4, a += f,   \
    g += h, g ^= h >>  8, b += g,   \
    h += a, h ^= a >>  9, c += h,   \
    a += b                          )


/* The base ISAAC initialization pass */
static void
isaac_mix(struct isaac_state* s, uint32_t seed[/*ISAAC_WORDS*/])
{
    int i;
    uint32_t a = s->iv[0];
    uint32_t b = s->iv[1];
    uint32_t c = s->iv[2];
    uint32_t d = s->iv[3];
    uint32_t e = s->iv[4];
    uint32_t f = s->iv[5];
    uint32_t g = s->iv[6];
    uint32_t h = s->iv[7];

    for(i = 0; i < ISAAC_WORDS; i += 8)
    {
        a += seed[i];
        b += seed[i + 1];
        c += seed[i + 2];
        d += seed[i + 3];
        e += seed[i + 4];
        f += seed[i + 5];
        g += seed[i + 6];
        h += seed[i + 7];

        mix(a, b, c, d, e, f, g, h);

        s->mm[i] = a;
        s->mm[i + 1] = b;
        s->mm[i + 2] = c;
        s->mm[i + 3] = d;
        s->mm[i + 4] = e;
        s->mm[i + 5] = f;
        s->mm[i + 6] = g;
        s->mm[i + 7] = h;
    }

    s->iv[0] = a;
    s->iv[1] = b;
    s->iv[2] = c;
    s->iv[3] = d;
    s->iv[4] = e;
    s->iv[5] = f;
    s->iv[6] = g;
    s->iv[7] = h;
}

/* Add a buffer of seed material */
static void isaac_seed_data(struct isaac_state* s, void* buffer, size_t size)
{
    unsigned char* buf = buffer;
    unsigned char* p;
    size_t avail;
    size_t i;

    avail = sizeof s->mm - s->c;    /* s->c is used as a write pointer */

    /* Do any full buffers that are necessary */
    while(size > avail)
    {
        p = (unsigned char*)s->mm + s->c;
        for(i = 0; i < avail; i++)
            p[i] ^= buf[i];
        buf += avail;
        size -= avail;
        isaac_mix(s, s->mm);
        s->c = 0;
        avail = sizeof s->mm;
    }

    /* And the final partial block */
    p = (unsigned char*)s->mm + s->c;
    for(i = 0; i < size; i++)
        p[i] ^= buf[i];
    s->c = size;
}

/* Initialize *S to a somewhat-random value.  */
static void
isaac_seed_start (struct isaac_state *s)
{
  static uint32_t const iv[8] =
    {
      0x1367df5a, 0x95d90059, 0xc3163e4b, 0x0f421ad8,
      0xd92a4a78, 0xa51a3c49, 0xc4efea1b, 0x30609119
    };

#if 0
  /* The initialization of iv is a precomputed form of: */
  int i;
  for (i = 0; i < 7; i++)
    iv[i] = 0x9e3779b9;		/* the golden ratio */
  for (i = 0; i < 4; ++i)	/* scramble it */
    mix (iv[0], iv[1], iv[2], iv[3], iv[4], iv[5], iv[6], iv[7]);
#endif

  memset (s->mm, 0, sizeof s->mm);
  memcpy (s->iv, iv, sizeof s->iv);

  /* s->c gets used for a data pointer during the seeding phase */
  s->a = s->b = s->c = 0;
}

/* End of seeding phase; get everything ready to produce output */
static void
isaac_seed_finish(struct isaac_state* s)
{
    isaac_mix(s, s->mm);
    isaac_mix(s, s->mm);
    /* Now reinitialize c to start things off right */
    s->c = 0;
}

#define ISAAC_SEED(s, x) isaac_seed_data (s, &(x), sizeof (x))

/* Initialize *S to a somewhat-random value; this starts seeding,
   seeds with somewhat-random data, and finishes seeding */
void isaac_seed(struct isaac_state* s)
{
    isaac_seed_start(s);

    { pid_t t = getpid();   ISAAC_SEED (s, t); }
    { pid_t t = getppid();  ISAAC_SEED (s, t); }
    { uid_t t = getuid();   ISAAC_SEED (s, t); }
    { gid_t t = getgid();   ISAAC_SEED (s, t); }

    {
        xtime_t t = gethrxtime();
        ISAAC_SEED(s, t);
    }

    isaac_seed_finish(s);
}
