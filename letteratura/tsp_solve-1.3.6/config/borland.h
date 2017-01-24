/* config.h
 *
 * orig ceh 5-1-94
 */
#ifndef _CONFIG_H
#define _CONFIG_H

/*
#define USE_SIGNALS
#define HAS_NFSLOCK
 */

#define HAS_FTIME
/*
#define HAS_FTIME_DECL
#define HAS_CLOCK
#define HAS_CLOCK_DECL
#define HAS_RAND_DECL
#define HAS_SPRINTF_DECL
 */
#define HAS_SRAND_DECL
#define HAS_TIME_DECL

#define REGSIGTYPE void

/*
#define HAS_AINT
 */

#include <stdlib.h>
#define MAX_RAND RAND_MAX

/* Borland 4.0 works just fine with "ptr = new Class[N];" but Borland 3.0 needs
 * a special define in tour.cc when compiling so it knows to construct N
 * instances of class Class.
#define NO_ARRAY_CONSTRUCTING
 */

#ifdef __cplusplus
extern "C" {
#endif
#ifndef HAS_RAND_DECL
extern int _Cdecl rand(void);
#endif
void _FAR * _Cdecl _FARFUNC memcpy(void _FAR *__dest,
   const void _FAR *__src, size_t __n);
#ifdef __cplusplus
}
#endif

#endif
