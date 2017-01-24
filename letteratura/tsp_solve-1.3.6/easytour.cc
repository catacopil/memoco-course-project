/* easytour.cc
 *
 * orig ceh
 */

/* If defined then the random tour returned is freshly seeded each time the
 * algorithm is called, otherwise the algorithm is seeded with time(0) each
 * invokation which will give the same tours as long as the algorithm is
 * invoked within the same second period.
 */
#define NEWLYRANDOM

/* seed with the standard seed give, so you get the same random tour everytime
 */
#define SAMERANDOM

#include "easytour.h"
#include <assert.h>
#include "chnew.h"
#include <math.h>
#include <time.h>
#include "params.h"
#include "rand.h"

#ifndef HAS_TIME_DECL
extern "C" {
   extern time_t time(time_t*);
}
#endif

#if !defined(SAMERANDOM) && defined(NEWLYRANDOM)
static unsigned randseed=0; 
#endif

int EasyTourHeuristic :: run()
{
   return 0;
}

int EasyTourHeuristic :: can_run(const Matrix *m) const
{
   return m!=NULL;
}

/* Build a random tour
 */
EasyTourHeuristic::EasyTourHeuristic (const Matrix *m, int ra) : TourFinder(m)
{
   city_id_t i, tmp, *array, ran;

   if (ra) {
      array = new city_id_t[degree];
      for (i=0; i<degree; i++)
         array[i] = i;
#ifdef SAMERANDOM
      param.reseed();
#else
# ifdef NEWLYRANDOM
      if (randseed++ == 0)
         srand(randseed = (unsigned)time(0));
      else
         srand(randseed);
# endif
#endif
      for (i=0; i<degree; i++) {
         ran = degree-1-(rand()%(degree-i));
         tmp = array[ran];
         array[ran] = array[i];
         tour->travel(tmp);
      }
#ifdef NEWLYRANDOM
      param.reseed();
#endif
      delete array;
   }
   else {
      for (i=0; i<degree; i++)
         tour->travel(i);
   }
}

/* Build an 'easy' tour from the order of cities already in the matrix
 */
EasyTourHeuristic::EasyTourHeuristic (const Matrix *m, Tour *t, int ra)
 : TourFinder(m)
{
   city_id_t i, tmp, *array, ran;
   TourIter ti(*t);

   if (ra) {
      array = new city_id_t[degree];
      for (i=0; i<degree; i++)
         array[i] = ti.next()->id;
#ifdef SAMERANDOM
      param.reseed();
#else
# ifdef NEWLYRANDOM
      if (randseed++ == 0)
         srand(randseed = (unsigned)time(0));
      else
         srand(randseed);
# endif
#endif
      for (i=0; i<degree; i++) {
         ran = degree-1-(rand()%(degree-i));
         tmp = array[ran];
         array[ran] = array[i];
         tour->travel(tmp);
      }
#ifdef NEWLYRANDOM
      param.reseed();
#endif
      delete array;
   }
   else
      tour->copy(*t);
}
