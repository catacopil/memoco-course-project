/* findtour.h
 *
 * to choose a tour finder from params
 *
 * orig ceh 12-25-93
 */
#ifndef _FINDTOUR_H
#define _FINDTOUR_H

#include "stdmacro.h"
#include "tour.h"
#include "matrix.h"
#include "tourfind.h"

// a timer for placing in the procedural memory in recursive calls that need
// to take care of Suspend Signal time stoping and keeping track of one
// tamer passed to the constructor.
class Stack_Timer
{
   Timer suspend_timer, *save_ptr, *add_timer;
   char quited, started;
public:
   char top_of_stack;
   Stack_Timer(Timer *add);
   // must be quit before destructor is called
   void quit(TourFinder *t);
   ~Stack_Timer();
};

// this will invoke a tsp finder by the abreviaiton given and
// also return the tour found from the invokation and add the time
// it takes to find te tour to the Timer passed.
//
// The additional include parameter is to call with 0 if you just want
// to find the length of an algorithms tour and not include the run in the
// total statistics tallyed by this class.  In addition, the include parameter
// also represents if you are running the FindTour constructor from another
// tour finder (if zero), or if the FindTour constructor is run from a main
// invokation of tsp_solve.  This also sets the param.invoked feature.
// 
// If a Tour is passed then, start improving it with the first improver passed
// with abrev
class FindTour {
   void write();
public:
   Tour *tour;
   sum_t length;
   FindTour(const char *abrev, const Matrix *, Timer *, int include, Tour *);
   ~FindTour();
};

#endif
