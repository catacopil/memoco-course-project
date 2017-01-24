/* CHalloc.h
 *
 * Inspired by Clint Staley's SmartAlloc.
 *
 * To use these automatic garbage collection/error detection routines on
 * malloc and free, #define MDEBUG and use malloc and free as you normally
 * would.  In addition these provide blocked malloc calls, i.e. when malloc
 * is called a great many times, it only allocates an average of 8 extra
 * bytes per malloc.  Also, blocked malloc calls and are faster to free
 * because allocations are grouped into less malloc() system calls.
 *
 * orig ceh 5-11-93
 */
#ifndef _CHALLOC_H
#define _CHALLOC_H

#ifdef MDEBUG

#ifdef __cplusplus
extern "C" {
#endif

#define __malloc_h /* used for portability with <malloc.h> */
#define _MALLOC_H

/*
#define DEFAULT_FILL_LONG 0x0L
#define DEFAULT_FILL ((char)0x0)
 */
#define DEFAULT_FILL_LONG 0xa0a0a0aL
#define DEFAULT_FILL ((char)0xa)

#ifdef malloc
#undef malloc
#endif
#define malloc(_byts) challoc(_byts,__FILE__,__LINE__,DEFAULT_FILL)
#ifdef calloc
#undef calloc
#endif
#define calloc(_itms,_byts) challoc((_byts)*(_itms),__FILE__,__LINE__,0)
extern void *challoc
   (const unsigned int bytes, const char *file, const int line,
   const char fill);

#ifdef free
#undef free
#endif
#define free(_ptr) chfree((_ptr),__FILE__,__LINE__)
extern void chfree (void *data, const char *file, const int line);

#ifdef realloc
#undef realloc
#endif
#define realloc(_ptr,_byts) chrealloc(_ptr,_byts,__FILE__,__LINE__,0xb)
extern void *chrealloc
   (void *data, const unsigned int bytes,
   const char *file, const int line, const int fill);

/* check_bounds()
 * sparsly call check_bounds() where you think you might be corrupting
 * unowned data to find specifically where you have corrupted unowned
 * data.
 */
#define check_bounds() _report_space(-1,__FILE__,__LINE__)

/* reports_space(kind_of_report)
 *
 * call report_space(0) to report the amount of memory used to stderr.
 * call report_space(1) to give a more verbose detail of memory usage.
 * report_space (1) will print out the installed block malloc status.
 * call report_space(2) to give a more detailed report of memory
 * used, it prints out a less verbose report than (3).  report_space (3)
 * will print a very detailed report of the location, where called,
 * and size of every single malloc was malloced.
 */
#define report_space(_prm) _report_space(_prm,__FILE__,__LINE__)
extern void _report_space
   (const int parm, const char *file, const int line);

/* install_blocked_mallocs(sizeof(type), items_per_block)
 *
 * If you find you have a lot of small mallocs and you find that freeing
 * is very slow, try the install_blocked_mallocs().  This will group
 * a number of the small allocations into one block of allocations so
 * internally only a few free() calls are called for many allocations.
 * The parameters of install_blocked_mallocs() are: the size of the memory
 * spaces you wish to "block" and a number of those items you wish to
 * put in a block to be freed at once if each one is called chfree() on.
 *
 * Example:
 *
 * long *array[1000];
 * install_blocked_mallocs(sizeof(long), 100);
 * for (x=0; x<1000; x++)
 *   array[x] = malloc(sizeof(long));
 */
#ifndef install_blocked_mallocs
#define install_blocked_mallocs(_s_,_i_) \
   _install_blocked_challoc(_s_,_i_,__FILE__,__LINE__)
/*
#define install_blocked_mallocs(_s_,_i_)
 */
#endif
extern void _install_blocked_challoc
   (unsigned int space, unsigned short items_per_block,
   const char *file, const int line);

#ifdef __cplusplus
};
#endif

#else /* MDEBUG */

#define report_space(_a_)
#define check_bounds()
#define install_blocked_mallocs(_s_,_i_)
#include <stdlib.h>

#endif /* MDEBUG */

#endif /* _CHALLOC_H */
