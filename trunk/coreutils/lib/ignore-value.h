/* ignore a function return without a compiler warning */

/* Use these functions to avoid a warning when using a function declared with
   gcc's warn_unused_result attribute, but for which you really do want to
   ignore the result. Traditionally, people have used a "(void)" cast to
   indicate that a function's return value is deliberately unused. However,
   if the function is declared with __attribute__((warn_unused_result)),
   gcc issues a warning even with the cast.

   Caution: most of the time, you really should heed gcc's warning, and 
   check the return value. However, in those exceptional cases in which
   you're sure you know what you're doing, use this function.

   For the record, here's one of the ignorable warnings:
   "copy.c:233: warning: ignoring return result of 'fchown',
   declared with attribute warn_unused_result". */

static inline void ignore_value(int i) { (void) i; }
static inline void ignore_ptr(void* p) { (void) p; }
