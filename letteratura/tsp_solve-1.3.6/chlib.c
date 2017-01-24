/* chlib.c
 *
 * orig ceh
 */

#include "config.h"
#include "chlib.h"
#include "challoc.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "stdmacro.h"

#define LOCKTIMEOUT 120
#define LOCKSLEEP 2

#ifdef __cplusplus
extern "C" {
#endif
#ifdef HAS_UNLINK_DECL
extern int unlink(const char *);
#endif
#ifndef HAS_SLEEP_DECL
extern int sleep(unsigned);
#endif
#ifndef HAS_CLOSE_DECL
extern int close(int);
#endif
extern int fprintf(FILE*, const char *, ...);

#ifndef sqr
#define sqr(_a) ((_a)*(_a))
#endif
#ifndef NULL
#define NULL 0
#endif

/* order this array so "string" would come before "*string*", the nums don't
 * have to be in sequence.
 */
static dname_type dname_array[]=
{
   {"EUC_2D", euc2d_distance,   0},
   {"EST",    est_distance,     1},
   {"ATT",    att_distance,     2},
   {"GEO",    geo_distance,     3},
   {"HOTO",   hoto_distance,    4},
   {"K",      knight_distance,  5},

   {"EXPLICIT", explicit_distance, -1}
};

dname_type* find_dname_from_num(const int n)
{
   dname_type *dn = dname_array;
   for (dn = dname_array; dn->num != -1; dn++)
      if (dn->num == n)
         break;
   return dn;
}

dname_type* find_dname_from_dist(const dist_f d)
{
   dname_type *dn = dname_array;
   for (dn = dname_array; dn->num != -1; dn++)
      if (dn->dist == d)
         break;
   return dn;
}

dname_type* find_dname_from_name(const char *nm)
{
   dname_type *dn = dname_array;
   for (dn = dname_array; dn->num != -1; dn++)
      if (strstr(nm, dn->name))
         break;
   return dn;
}

#ifndef HAS_AINT
#define aint(_x) ((double)((long)(_x)))
#endif

double explicit_distance(const pos_t *p1, const pos_t *p2)
{
   return aint(sqrt(sqr(p1->x-p2->x)+sqr(p1->y-p2->y))+.5);
}

double euc2d_distance(const pos_t *p1, const pos_t *p2)
{
   return aint(sqrt(sqr(p1->x-p2->x)+sqr(p1->y-p2->y))+.5);
}

double est_distance(const pos_t *p1, const pos_t *p2)
{
   return sqrt(sqr(p1->x-p2->x)+sqr(p1->y-p2->y));
}

double att_distance(const pos_t *p1, const pos_t *p2)
{
   double rij, tij;
   rij = sqrt( (sqr(p1->x-p2->x)+sqr(p1->y-p2->y)) / (double)10.0 );
   tij = aint( rij );
   if (tij < rij)
      return tij + (double)1.;
   return tij;
}

static void compute_lat_lon(const pos_t *p1, pos_t *l1)
{
    double math_pi = 3.141592, deg, min;
    deg = aint( p1->x );
    min = p1->x - deg;
    l1->x = math_pi * (deg + 5. * min / 3.) / 180.;
    deg = aint( p1->y );
    min = p1->y - deg;
    l1->y = math_pi * (deg + 5. * min / 3.) / 180.;
}

double geo_distance(const pos_t *p1, const pos_t *p2)
{
   pos_t l1, l2;
   double RRR = 6378.388, q1, q2, q3;

   compute_lat_lon(p1, &l1);
   compute_lat_lon(p2, &l2);

   q1 = cos( l1.y - l2.y );
   q2 = cos( l1.x - l2.x );
   q3 = cos( l1.x + l2.x );
   return floor( (RRR * acos( 0.5*((1.0+q1)*q2-(1.0-q1)*q3) )) + 1.0);
}

double hoto_distance(const pos_t *p1, const pos_t *p2)
{
   double xd, yd;
   xd = abs(p1->x - p2->x);
   yd = abs(p1->y - p2->y);
   xd = sqrt(xd * xd + yd * yd) / 7.69;
   /*
   return xd;
    */
   return (long)xd;
}

#ifdef strncmp
int chstrncmp(const char *ss1, const char *ss2, const int l)
{
   char *s1 = (char*)ss1, *s2 = (char*)ss2, *end = s1+l;
   if (l == 0)
      return 0;
   while (end != s1 && *s1++ == *s2++)
      ;
   return ((unsigned)*--s1-(unsigned)*--s2);
}
#endif

#ifdef strcpy
char *chstrcpy(char *ss1, const char *ss2)
{
   char *s2 = (char*)ss2, *s1 = ss1;
   while ((*s1++ = *s2++) != 0)
      ;
   return ss1;
}
#endif

#ifdef strlen
int chstrlen(const char *ss1)
{
   char *s1 = (char*)ss1;
   while (*s1++ != 0)
      ;
   return (int)(s1-(char*)ss1)-1;
}
#endif

#ifdef strcat
char *chstrcat(char *s1, const char *ss2)
{
   char *ss1 = s1, *s2 = (char*)ss2;
   while (*s1++ != 0)
      ;
   for (s1--; (*s1++ = *s2++) != 0; )
      ;
   return ss1;
}
#endif

#ifdef strcmp
int chstrcmp(const char *ss1, const char *ss2)
{
   char *s2 = (char*)ss2, *s1 = (char*)ss1;
   while (*s1 == *s2 && *s1 != 0 && *s2 != 0)
      s1++, s2++;
   return (*s1 - *s2);
}
#endif

#ifdef strstr
char *chstrstr(const char *ss, const char *ss2)
{
   char *s1 = (char*)ss, *s2, *t;

   while (*s1 != 0)  {
      if (*s1 == *ss2) {
         for (t = s1+1, s2 = (char*)ss2+1; *t != 0 && *s2 != 0 && *t == *s2; )
            t++, s2++;
         if (*s2 == 0)
            return s1;
      }
      s1++;
   }
   return (*ss2 == 0) ? s1 : NULL;
}
#endif

char *lockfile(const char *file, int r)
{
#ifdef HAS_NFSLOCK
   extern long time(unsigned);
   char *lockname, *tmp, *newtmp;
   int len = strlen(file), fd, i, sleepytime = r?(((time(0)/r)%3)+1):LOCKSLEEP;

   lockname = (char*)malloc(len+4);
   strcpy(lockname, file);

   for (tmp = lockname+len; --tmp > lockname; )
      if (tmp[-1] == '/')
         break;
   for (newtmp = lockname+len; newtmp >= tmp; newtmp--)
      newtmp[3] = *newtmp;
   tmp[0] = '.';
   tmp[1] = '_';
   tmp[2] = '.';

   for (i = 0; (fd = open(lockname, O_CREAT|O_EXCL)) == -1 && i<LOCKTIMEOUT; )
      sleep(sleepytime), i++;
   if (fd != -1) {
      close(fd);
   }
   else {
      extern char* ctime(long *);
      long t = time(0);
      fprintf(stdout, "ERROR : Failed to open %s! %s", lockname, ctime(&t));
      free(lockname);
      lockname = NULL;
   }
   return lockname;
#else
   return NULL;
#endif
}

#include <errno.h>
void unlockfile(char *lockname)
{
   if (lockname != NULL) {
#ifdef HAS_NFSLOCK
      if (unlink(lockname) == -1)
         fprintf(stdout, "'%s' unlink errno %d\n", lockname, errno);
#endif
      free(lockname);
   }
}

float sizeof_array[] = {
   sizeof(int), sizeof(city_id_t), sizeof(cost_t), sizeof(sum_t),
#ifdef FLOAT_COST
   1.,
#else
   2.,
#endif
   MAX_COST, MAX_SUM, 0.
};

#define SMALL_ARRAY_SIZE 11
double knight_distance(const pos_t *p1, const pos_t *p2)
{
   int xdiff, ydiff, diff, odd, dist;
   static int small_dist[] = { 2, 3, 2, 3, 2, 3, 4, 5, 4, 5, 6, 3, 2, 1,
   2, 3, 4, 3, 4, 5, 6, 5, 2, 1, 4, 3, 2, 3, 4, 5, 4, 5, 6, 3, 2, 3, 2,
   3, 4, 3, 4, 5, 6, 5, 2, 3, 2, 3, 4, 3, 4, 5, 4, 5, 6, 3, 4, 3, 4, 3,
   4, 5, 4, 5, 6, 5, 4, 3, 4, 3, 4, 5, 4, 5, 6, 5, 6, 5, 4, 5, 4, 5, 4,
   5, 6, 5, 6, 7, 4, 5, 4, 5, 4, 5, 6, 5, 6, 7, 6, 5, 6, 5, 6, 5, 6, 5,
   6, 7, 6, 7, 6, 5, 6, 5, 6, 5, 6, 7, 6, 7, 8 };

   xdiff = (p1->x - p2->x);
   ydiff = (p1->y - p2->y);
   if (xdiff < 0)
      xdiff *= -1;
   if (ydiff < 0)
      ydiff *= -1;
   if (xdiff > ydiff) {
      diff = xdiff;
      xdiff = ydiff;
      ydiff = diff;
   }
   odd = (xdiff + ydiff) % 2;
   assert(ydiff >= xdiff);
   if (ydiff < SMALL_ARRAY_SIZE) {
      if (p1->x == p1->y && p2->x == p2->y && xdiff == 1
       && (p1->x == 0 || p2->x == 0))
         dist = 4;
      else
         dist = small_dist[xdiff*SMALL_ARRAY_SIZE + ydiff];
   }
   else if (!(diff = ydiff - 2*xdiff))
      dist = xdiff;
   else if (diff > 0.) {
      int seg1 = (ydiff - 1) / 2 + 1, seg2;
      if (seg1 % 2)
         seg2 = seg1 + 1;
      else {
         seg2 = seg1;
         seg1 = seg2 + 1;
      }
      if (odd)
         dist = seg1;
      else
         dist = seg2;
   }
   else {
      if (odd)
         dist = ((xdiff + (ydiff - xdiff) / 2 + 1) / 3) * 2 + 1;
      else
         dist = ((xdiff + (ydiff - xdiff) / 2 - 1) / 3) * 2 + 2;
   }
   return dist;
}

#ifdef __cplusplus
};
#endif
