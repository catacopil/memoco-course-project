/* chlib.h
 *
 * extra library functions that compile with less errors than trying to find
 * externs for other library functions, plus some miscelaneous distance
 * functions.
 */
#ifndef _CHLIB_H
#define _CHLIB_H

#include <stdlib.h> /* mostly for size_t */
#include <math.h>
/*
#include <string.h>
*/
#define strncmp chstrncmp
#define strcmp chstrcmp
#define strlen chstrlen
#define strstr chstrstr
#define strcpy chstrcpy
#define strcat chstrcat

#define MAX_POS 1000.0

typedef struct  {
   double x, y;
} pos_t;

#ifdef __cplusplus
extern "C" {
#endif

typedef double (*dist_f)(const pos_t*, const pos_t*);

typedef struct {
   const char *name;
   dist_f dist;
   int num;
} dname_type;

extern dname_type *find_dname_from_dist(const dist_f d);
extern dname_type *find_dname_from_num(const int n);
extern dname_type *find_dname_from_name(const char *nm);

extern int chstrncmp(const char *, const char *, const int);
extern int chstrcmp(const char *, const char *);
extern int chstrlen(const char *);
extern char* chstrstr(const char *, const char *);
extern char* chstrcpy(char *, const char *);
extern char* chstrcat(char *, const char *);
extern double att_distance(const pos_t *p1, const pos_t *p2);
extern double est_distance(const pos_t *p1, const pos_t *p2);
extern double euc2d_distance(const pos_t *p1, const pos_t *p2);
extern double geo_distance(const pos_t *p1, const pos_t *p2);
extern double hoto_distance(const pos_t *p1, const pos_t *p2);
extern double knight_distance(const pos_t *p1, const pos_t *p2);
/* asserts that this shouldn't be used */
extern double explicit_distance(const pos_t *p1, const pos_t *p2);
#define default_distance est_distance

/* utilities to lock a file from being used twice.  If the lockfile exists
 * then this function attempts to retry after a timeout.  If the r parameter
 * is nonzero then the timeout to wait to retry, becomes (time(0)/r)%3.
 */
extern char *lockfile(const char *, int r);
extern void unlockfile(char *);

/* an array terminated by a zero of the sizeof()'s of non-portable datatypes,
 * also includes the MAX sizes of the data types.
 */
extern float sizeof_array[];

#ifdef __cplusplus
};
#endif

#endif
