/* Detect the number of processors. */

#ifndef _NPROC_H
#define _NPROC_H

/* Allow the use of C++ code */
#ifdef __cplusplus
extern "C" {
#endif

/* A "processor" in this context means a thread execution unit, that is either
   -- an execution core in a (possibly multi-core) chip, in a (possibly multi-
      chip) module, in a single computer, or
   -- a thread execution unit inside a core
   Wich of the two definitions is used, is unspecified. */

enum nproc_query
{
    NPROC_ALL,                  /* total number of processors */
    NPROC_CURRENT,              /* processors available to the current process */
    NPROC_CURRENT_OVERRIDABLE   /* likewise, but overridable throught the
                                   OMP_NUM_THREADS evironment variable */
};

/* Return the total number of processors. The result is guaranteed to
   be at least 1. */

extern unsigned long int num_processors(enum nproc_query query);

#ifdef __cplusplus
}
#endif /* C++ */

#endif
