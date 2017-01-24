/* solver.cc
 *
 * base class tsp solver
 * orig ceh
 */

#include "solver.h"
#include "stdmacro.h"
#include "parallel.h"

TSPSolver :: ~TSPSolver()
{
}

int TSPSolver :: run()
{
  dump << "DUMP RUN\n";
  return 100;
}

void TSPSolver :: save_solver(BinFile &file) const
{
   TourFinder::save(file);
   file << Initial_Lower_Bound << best_so_far;
}

void TSPSolver :: load_solver(BinFile &file)
{
   TourFinder::load(file);
   file >> Initial_Lower_Bound >> best_so_far;
}

void TSPSolver :: update_tour(Tour *newtour, int tell_others)
{
   sum_t newbest = newtour->cost(matrix), last = best_so_far;
   if (newbest < best_so_far && newtour->is_complete(matrix->degree)) {
      best_so_far = newbest;
      if (newtour != tour)
         tour->copy(*newtour);
   }
   parallel.update_new_tour(tour, best_so_far, tell_others, matrix);
   que.strip(best_so_far, clipper);
   if (param.verbose && last > best_so_far) {
      dump << "FOUND " << best_so_far << "\n";
      tour->print();
      dump.flush();
   }
}
