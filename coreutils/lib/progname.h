#ifndef _PROGNAME_H
#define _PROGNAME_H

#ifdef __cplusplus
extern "C" {
#endif

extern const char* program_name;
extern void set_program_name(const char* argv0);

#if  ENABLE_RELOCATABLE
extern void set_program_name_and_installdir(const char* argv0,
                                            const char* orig_installprefix,
                                            const char* orig_installdir);

#undef set_program_name
#define set_program_name(ARG0) \
            set_program_name_and_installdir(ARG0, INSTALLPREFIX, INSTALLDIR)

/* Return the full pathname of the current executable, based on the earlier
   call to set_program_name_and_installdir. Return NULL if unknown. */
extern char* get_full_program_name(void);

#endif /* ENABLE_RELOCATABLE */












#ifdef __cplusplus
}
#endif


#endif
