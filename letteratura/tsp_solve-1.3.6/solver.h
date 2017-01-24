/* solve.h
 * the base optimal tourfinder/solver
 * orig ceh
 */


#ifndef _TSPSOLVE_H
#define _TSPSOLVE_H

#include "tourfind.h"
#include "pqueue.h"

class TSPSolver : public TourFinder
{
protected:
   void *current_split;
   PQueue que;
   void (*clipper)(PQuelem*);
public:
   // the length of the best solution so far
   sum_t best_so_far, Initial_Lower_Bound;
   TSPSolver (const Matrix *m) : TourFinder (m)
   {
      clipper = NULL;
      Initial_Lower_Bound = 0;
      best_so_far = SUM_MAX;
      current_split = NULL;
   };
   virtual ~TSPSolver();
   /*
   virtual void write(Term &) const;
    */

   // run will return the percentage of the algorithm is yet to be done.
   // the TourFinder will be done when run returns 0 (nothing left to do.)
   // when run() returns 0, get_TourFound() can be called to get the TourFound.
   // run() still can return 0, but the TourFinder might have not found a Tour.
   // run will possibly modify the tour to be suited as the best tour so
   // far, even if the return is not yet 0.
   virtual int run();

   void load_solver(BinFile &);
   void save_solver(BinFile &) const;

   // updates a new tour into this solver and sets the appropriate so far
   // minimum best tour length, if a newtour passed is different than
   // the address of the TSPSolver's tour, then it is copied into the others
   // if the length is better than the origonal tour.  If tell_others is
   // zero, then the tour is already known by other parallel processes.
   // otherwise signals are sent to other parallel processes to check if their
   // tour is not as good as the new tour being updated here.
   //
   // clip is the procedure if it exists, to call when a better tour is better
   // than some of the elements left on the que; 
   void update_tour(Tour *newtour, int tell_others);
};

#endif
