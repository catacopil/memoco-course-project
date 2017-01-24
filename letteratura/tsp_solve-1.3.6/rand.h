/* rand.h
 *
 * to de defined when using a random function returning an undefined
 * maximum integer
 */
#ifndef _RAND_H
#define _RAND_H

#include "config.h"
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifndef HAS_RAND_DECL
extern int rand(void);
#endif
#ifndef HAS_LRAND48_DECL
extern long lrand48(void);
#endif
#ifndef HAS_SRAND_DECL
extern void srand(int);
#endif
#ifndef HAS_SRAND48_DECL
extern void srand48(int);
#endif
#ifdef __cplusplus
};
#endif

/* For replaing the useage of rand with the rand48 library
 */
#ifdef HAS_RAND48
# define rand lrand48
# define srand srand48
#endif

#endif
