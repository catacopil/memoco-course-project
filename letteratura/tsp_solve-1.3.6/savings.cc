/* savings.cc
 *
 * orig ceh 5-23-95
 */
/*
#define PRINTIT
 */

#include "savings.h"
#include <assert.h>
#include "chnew.h"
#include "params.h"
#include "smatrix.h"

int SavingsHeuristic :: run()
{
   return 0;
}

void SavingsHeuristic :: print(Tour **tours)
{
   city_id_t i;
   for (i = 0; i < degree; i++) {
      dump << "Tour " << i << ": ";
      tours[i]->print();
   }
}

int SavingsHeuristic :: can_run(const Matrix *m) const
{
   return m!=NULL;
}

/* the status array is so that it points to the tour the index is on.
 * if the best edge is i to j, then the status[i] wil point to the tour which
 * i is tail, and status[j] points to the tour which j is head.
 */
SavingsHeuristic::SavingsHeuristic (const Matrix *m, int) : TourFinder(m)
{
   city_id_t i, newi, j, start = ((param.initial_choice == NO_ID)
    ? 0 : param.initial_choice), x, k, l, *status, *status2;
   SortedMatrix sm(degree);
   SortedCost *sc, **scsave = NULL, **ssc;
   SortedCost **scarray = new SortedCost*[degree], **sscend = scarray+degree;
   Tour **tours = new Tour*[degree], *ti = NULL, *tj;
   status2 = (status = new city_id_t[degree*2]) + degree;
   lambda_cost_t best;
   int sym = matrix->is_symmetric();

   sm.sort(m, start);
   for (i = 0; i < degree; i++) {
      status2[i] = status[i] = i;
      tours[i] = new Tour;
      tours[i]->travel(i);
      scarray[i] = sm.cost[i];
   }
#ifdef PRINTIT
   m->print();
   sm.print();
   print(tours);
#endif
   for (x = 0; x < degree-2; x++) {
      best = MAX_LCOST;
      for (ssc = scarray; ssc < sscend; ssc++) {
         switch (i = status[ssc-scarray]) {
         default: if (i != start) break;
         case NO_ID: case MAX_DEGREE: continue;
         }
         sc = *ssc;
         while ((sym ? status[sc->id] == MAX_DEGREE :
          !(status[sc->id] == NO_ID || status[sc->id] == sc->id))
          || i == sc->id) {
            assert(sc->real_cost != MAX_COST);
            *ssc = ++sc;
         }
         if (best > sc->cost) {
            scsave = ssc;
            best = sc->cost;
         }
      }
      assert(best != MAX_LCOST);
      tj = tours[k = status2[j = (*scsave)->id]];
      ti = tours[l = status2[i = (city_id_t)(scsave-scarray)]];
#ifdef PRINTIT
dump << "Adding [" << i << "," << j << "]\n";
#endif
      assert(status[i] != NO_ID && status[i] != MAX_DEGREE);
      assert(status[j] != MAX_DEGREE);
      status2[status[i]] = k;
      ti->append(tj);
      if (status[i] == i) {
         status[i] = NO_ID;
         newi = i;
      }
      else {
         newi = status[i];
         status[i] = MAX_DEGREE;
      }
      if (status[j] == NO_ID) {
         status[k] = newi;
         status[j] = MAX_DEGREE;
      }
      else if (status[j] == j)
         status[j] = newi;
      else {
         assert(sym);
         assert(status[status[j]] == NO_ID);
         tours[k] = tours[status[j]];
         k = status2[status[j]] = status2[newi] = status[j];
         status[status[j]] = newi;
         status[j] = MAX_DEGREE;
         tj->reverse();
      }
      tours[k] = ti;
      tours[l] = tj;
#ifdef PRINTIT
print(tours);
#endif
   }
   ti->append(tours[start]);
   tour->copy(*ti);
   for (i = 0; i < degree; i++)
      delete tours[i];
   delete scarray;
   delete status;
   delete tours;
}
