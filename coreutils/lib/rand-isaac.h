/* Bob Jenkins's cryptographic random number generator, ISAAC */

#ifndef RAND_ISAAC_H
#define RAND_ISAAC_H

#include <stdint.h>

/* Size of the state tables to use. ISAAC_LOG should be at least 3,
   and smaller values give less security */
#define ISAAC_LOG 8
#define ISAAC_WORDS (1 << ISAAC_LOG)
#define ISAAC_BYTES (ISAAC_WORDS * sizeof(uint32_t))

/* RNG state variables. The members of this structure are private */
struct isaac_state
{
    uint32_t mm[ISAAC_WORDS];   /* Main state array */
    uint32_t iv[8];             /* Seeding initial vector */
    uint32_t a, b, c;           /* Extra index variables */
};

void isaac_seed(struct isaac_state* s);
void isaac_refill(struct isaac_state* s, uint32_t r[]);


#endif
