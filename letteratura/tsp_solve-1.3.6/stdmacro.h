/* stdmacro.h
 * some standard macros and cost typedefinitions
 * orig ceh
 */
#ifndef _STDMACRO_H
#define _STDMACRO_H

#include <stdlib.h> /* mostly for size_t */
#include <assert.h>
#ifdef __cplusplus
#include "chnew.h"
#include "io.h"
#endif
#include "config.h"

/* if cost_t is to be floating point
 */
#define FLOAT_COST

/* if city_id_t is not to be signed
#define UNSIGNED_CITY_ID
 */

/* if city_id_t is to be two bytes
 */
#define LARGE_CITY_ID

#ifndef FLOAT_MAX
# define FLOAT_MAX (1e30)
#endif
#ifndef FLOAT_MIN
# define FLOAT_MIN (-1e30)
#endif

typedef unsigned char _uchar;

#define isbetween(_a,_b,_c) ((_a)>=(_b) && (_a)<=(_c))
#ifndef max
# define max(_a,_b) (((_a)>(_b))?(_a):(_b))
#endif
#define strduup(_a,_b) strcpy(new char[strlen(_a)+1+(_b)], _a)
#define fequiv(_a,_b,_c) (fabs((double)(_a)-(_b))<(double)(_c))

/* die_if() is an always Forced assertion
 */
#ifndef NDEBUG
# define die_if(_a, _b, _c) if (_a) dump \
   << (_b) << " : " << (_c) << "\n", assert(!(_a))
#else
# define die_if(_a, _b, _c) if (_a) dump \
   << "Failed Forced Condition on " << __FILE__ << ":" << __LINE__ << "\n" \
   << (_b) << " : " << (_c) << "\n", exit(1)
#endif

/* MIN and MAX costs or sums relate values that real matrix costs or sums of
 * matrix costs cannot be.
 */
#ifdef FLOAT_COST
# define cost_t      float
# define sum_t       double
# define COST_MAX    ((cost_t)1414000L)
# define COST_MIN    ((cost_t)-1414000L)
/*
# define COST_MAX    ((cost_t)1e7)
# define COST_MIN    ((cost_t)-1e7)
 */
# define SUM_MAX     ((sum_t)1e10)
# define SUM_MIN     ((sum_t)-1e10)
# define SUM_GRANU   ((sum_t)(1./6.))
#else
/*
# define COST_MAX    ((cost_t)69999999L)
# define COST_MIN    ((cost_t)-69999999L)
# define SUM_MAX     ((sum_t)2000000000L)
# define SUM_MIN     ((sum_t)-2000000000L)
# define cost_t      long
 */
# define COST_MAX    ((cost_t)19999)
# define COST_MIN    ((cost_t)-19999)
# define SUM_MAX     ((sum_t)9999999L)
# define SUM_MIN     ((sum_t)-9999999L)
# define cost_t      short
# define SUM_GRANU   ((sum_t)1)
# define sum_t       long
#endif

#define MAX_COST COST_MAX
#define NO_COST COST_MAX
#define MAX_SUM SUM_MAX
#define MIN_SUM SUM_MIN
#define MAX_FLOAT FLOAT_MAX
#define MIN_FLOAT FLOAT_MIN
#define MIN_COST COST_MIN

typedef
#ifdef UNSIGNED_CITY_ID
unsigned
#else
signed
#endif
#ifdef LARGE_CITY_ID
short
#else
char
#endif
city_id_t;

/* signed_city_id_t is the same sizeof() as the city_id_t but can be negative
 * and should be asserted not to go more than positive MAX_DEGREE/2
 */
typedef
signed
#ifdef LARGE_CITY_ID
short
#else
char
#endif
signed_city_id_t;

#define NO_ID ((city_id_t)-1)

#ifndef NULL
# define NULL (void*)0
#endif

#define OLD_SIGNAL

#endif
