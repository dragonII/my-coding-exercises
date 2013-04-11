/* Convert string to double, using the C locale */

#ifndef LIB_C_STRTOD_H
#define LIB_C_STRTOD_H

/* Parse the initial portion of the string pointer to by NPTR as a floating-
   point number (in decimal or hexadecimal notation), like in the C locale:
   accepting only the ASCII digits '0'..'9', and only '.' as decimal point
   character.
   If ENDPTR is not NULL, set *ENDPTR to point to the first byte beyong the
   parse number or to NPTR if the string does not start with a parseable
   number.
   Return value:
   - If successful, return the value as a double or 'long double',
     respectively, and don't modify errno.
   - In case of overflow, return +/-HUGE_VAL or +/-HUGE_VALL, respectively, and
     set errno to ERANGE.
   - In case of underflow, return a value very near to 0 and set errno to
     ERANGE.
   - If the string does not start with a number at all, return 0 (and recall
     that it ENDPTR != NULL, *ENDPTR is set to NPTR), and maybe set errno to
     EINVAL.
   - In case of other error, return 0 and set errno, for example to ENOMEM. */

double c_strtod(char* nptr, char** endptr);
double c_strtold(char* nptr, char** endptr);

#endif
