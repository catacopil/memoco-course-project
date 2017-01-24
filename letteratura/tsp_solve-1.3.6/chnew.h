/* CHnew.h
 *
 * No don't say godblessyou.  This a C++ garbage collection utility.
 *
 * orig ceh Aug 93
 */
#ifndef _CHNEW_H
#define _CHNEW_H

#include <stdlib.h> // mostly for size_t
#include <stdio.h> // to cleanse possible new #defines

#ifdef MDEBUG

// hopefully private globals
extern char *__fileNAME;
extern int __lineNUM;

// Only for use in the overloaded delete macro, don't use on its own.
// The name stack is so that inherited destructor calls will preserve
// their filename and line number whilest recursively calling delete.
extern void* _push_dcall_stack(const char *filename, const int line);

// the overloaded new that uses garbage collection/decetion utilities
#ifdef new
#undef new
#endif
extern void *operator new(size_t size);
#define new ((__fileNAME = __FILE__) != __FILE__ \
 || (__lineNUM = __LINE__) != __LINE__) \
 ? 0 : new

// the overloaded delete that uses garbage collection/decetion utilities
#ifdef delete
#undef delete
#endif
extern void operator delete(void *ptr);
#define delete _push_dcall_stack(__FILE__, __LINE__), delete

// CHalloc extensions, for bounds checking and blocked mallocing
#define check_bounds() _report_space(-1, __FILE__, __LINE__)
#define report_space(_prm) _report_space(_prm, __FILE__, __LINE__)
extern "C" void _report_space(const int, const char *file, const int line);
#ifdef install_blocked_mallocs
#undef install_blocked_mallocs
#endif
/*
#define install_blocked_mallocs(_s_,_i_)
 */
#define install_blocked_mallocs(_s_, _i_) \
   _install_blocked_challoc(_s_, _i_, __FILE__, __LINE__)
extern "C" void _install_blocked_challoc
   (unsigned int space, unsigned int items_per_block,
   const char *file,const int line);

#else

#define report_space(_a_)
#define check_bounds()
#define install_blocked_mallocs(_s_, _i_)

#endif // MDEBUG
#endif // _CHNEW_H
