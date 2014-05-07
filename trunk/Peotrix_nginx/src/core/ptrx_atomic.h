#ifndef __PTRX_ATOMIC_H__
#define __PTRX_ATOMIC_H__

#include <stdint.h>

typedef int32_t                         ptrx_atomic_int_t;
typedef uint32_t                        ptrx_atomic_uint_t;
typedef volatile ptrx_atomic_uint_t     ptrx_atomic_t;

#define PTRX_ATOMIC_T_LEN   (sizeof("-2147483648") - 1)

static inline ptrx_atomic_uint_t
ptrx_atomic_cmp_set(ptrx_atomic_t *lock, ptrx_atomic_uint_t old,
                    ptrx_atomic_uint_t set)
{
    if(*lock == old)
    {
        *lock = set;
        return 1;
    }
    return 0;
}

inline void ptrx_memory_barrier() {}
inline void ptrx_cpu_pause() {}


#define ptrx_trylock(lock) (*(lock) == 0 && ptrx_atomic_cmp_set(lock, 0, 1))
#define ptrx_unlock(lock)   *(lock) = 0


#endif
