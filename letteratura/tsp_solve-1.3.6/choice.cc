/* choice.cc
 *
 * for all tour finder choices
 *
 * orig ceh
 */
#define USE_EQUAL

/* SPECIAL1
#define ITERS ((m->degree*2)/3)
#define BESTA (1+(m->degree)/20)
 */
/* SPECIAL2
#define ITERS (m->degree)
#define BESTA (1+(m->degree)/11)
 */
/* SPECIAL3
 */
#define ITERS ((m->degree*3)/2)
#define BESTA (1+(m->degree)/5)
/*
#define ITERS 50
#define BESTA 25
 */

#include "choice.h"
#include "findtour.h"
#include "standard.h"
#include "onetsolv.h"
#include "onetree.h"
#include "oneabor.h"
#include "addition.h"
#include "easytour.h"
#include "disperse.h"
#include "nopt.h"
#include "assign.h"
#include "multfrag.h"
#include "loss.h"
#include "hungaria.h"
#include "savings.h"
#include "flexmap.h"

choice choices[]=
{
   {"best", "Best Optimal", 0}, //0
   {"imponetree", "Improved 1-Tree", 0}, //1
   {"onetree", "B&B 1-Tree", 0}, //2
   {"assign", "Assignment B&B", 0}, //3
   {"standard", "B&B Least Arcs", 0}, //4

   {"heurbest", "BestHeuristic", 0}, //5
   {"psort", "Sorted Assignment Patching", 0}, //6
   {"addition", "Addition", 0}, //7
   {"farinsert", "Farthest Insertion", 0}, //8
   {"dispersion", "Dispersion", 0}, //9
   {"angle", "Angle Convex Addition", 0}, //10
   {"easytour", "Easy Tour", 0}, //11
   {"loss", "Loss Heuristic", 0}, //12
   {"quick", "Quick best", 0}, //13
   {"threeopt", "Three-opt", 0}, //14
   {"fouropt", "Assym Four-opt", 0}, //15
   {"k3aswap", "3-Swap Asymm", 0}, //16
   {"fiveopt", "Assym Five-opt", 0}, //17
   {"k4aswap", "4-Swap Asymm", 0}, //18
   {"k14aswap", "14-Swap Asymm", 0}, //19
   {"multifrag", "Multiple Fragment", 0}, //20
   {"bio", "Best In Out MultiFrag", 0}, //21
   {"k5", "5-Swap Symm", 0}, //22
   {"k17", "17-Swap Symm", 0}, //23
   {"split", "Split Farthest/Nearest Addition", 0}, //24
   {"ropt", "R-Opt", 0}, //25
   {"random", "Random Tour", 0}, //26
   {"christofide", "Christofides Heuristic", 0}, //27
   {"patching", "Assignment Patching", 0}, //28
   {"arbor", "1-Arborescence", 0}, //29
   {"hungarian", "Hungarian Algorithm", 0}, //30
   {"k10swap", "10-Swap", 0}, //31
   {"savings", "Savings Heuristic", 0}, //32
   {"flexmap", "FLEXMAP", 0}, //33
   {"r1", "Random after random", 0}, //34
   {"r2", "Random from start", 0}, //35

   {NULL, NULL, 0}
};

static char *best_base_tours[]=
{
   "ad+ro", "fa+ro", "e+ro", "di+ro", "mu+ro", NULL
};

/* Point *1*: if either the same tour is found in the stash, or we've found
 * more than a third of the stash as the same value then break.
 */
TourFinder *switch_tourfinders(short x, const Matrix *m)
{
   TourFinder *ret;
   Timer d;

   switch (x) {
   case 0:
      if (m->degree <= 12) {
         ret = new HungarianSolver(m);
      }
      else if (m->is_symmetric()) {
         ret = new OneTreeSolver(m);
      }
      else {
         if (m->degree <= 15)
            ret = new HungarianSolver(m);
         else
            ret = new APSolver(m);
      }
      break;
   case 1: ret = new OneTreeSolver(m); break;
   case 2: ret = new OTreeSolver(m); break;
   case 29: ret = new OneAborSolver(m); break;
   case 30: ret = new HungarianSolver(m); break;
   case 3: ret = new APSolver(m); break;
   case 4: ret = new Standard(m); break;
   case 34:
   case 35:
      {
      FindTour *tf = new FindTour("e", m, &d, 0, NULL);
      Tour tour(m->degree), tours_a[BESTA], temp_tour(m->degree), *tours[BESTA];
      Tour *best;
      sum_t bestcost=MAX_SUM, sums[BESTA], save_sum, sum;
      int i, uniq, startj;
      for (i = 0; i < BESTA; i++) {
         tours[i] = &tours_a[i];
         sums[i] = MAX_SUM;
      }
      temp_tour.copy(*tf->tour);
      delete tf;
      for (i = 0; i < ITERS; i++, (x == 35)
       ? temp_tour.copy(*tf->tour) : (void)0, delete tf) {
         tf = new FindTour("+ra+ro", m, &d, 0, &temp_tour);
         sum = tf->tour->cost(m);
         if (param.verbose > 1)
            dump << sum << "\n";
         if (sums[0] > sum) {
            int j;
            best = tours[0];
            save_sum = sums[0];
            for (startj = 0, uniq = j = 1; j < BESTA; j++) {
               if (sums[j] < sum)
                  break;
#ifdef USE_EQUAL
               else if (sums[j] == sum) {
						if (startj == 0)
							startj = j;
						if (startj+BESTA/3 < j || !tf->tour->compare(*tours[j])) {
                     uniq = 0; /*1*/
                     break;
                  }
                  sums[j-1] = sums[j];
                  tours[j-1] = tours[j];
						/*
                  break;
						 */
               }
#endif
               else {
                  sums[j-1] = sums[j];
                  tours[j-1] = tours[j];
               }
            }
#ifdef USE_EQUAL
            if (!uniq) {
               for (j--; j > 0; j--) {
                  sums[j] = sums[j-1];
                  tours[j] = tours[j-1];
               }
               tours[0] = best;
               sums[0] = save_sum;
               continue;
            }
#endif
            sums[j-1] = sum;
            (tours[j-1] = best)->copy(*tf->tour);
         }
      }
      for (i = BESTA; --i >= 0 && sums[i] != MAX_SUM; ) {
/*
#ifdef USE_EQUAL
 */
         if (param.verbose > 1)
            tours[i]->print();
/*
#endif
 */
         tf = new FindTour("+th+ro", m, &d, 0, tours[i]);
         sum = tf->tour->cost(m);
         if (param.verbose > 1)
            dump << sums[i] << " - " << sum << "\n";
         if (bestcost > sum) {
            bestcost = sum;
            temp_tour.copy(*tf->tour);
         }
         delete tf;
      }
      ret = new EasyTourHeuristic(m, &temp_tour, 0);
      }
      ret->duration += d;
      break;
   case 5:
      if (m->is_symmetric()) {
         FindTour *tf;
         Tour tour(m->degree);
         sum_t bestcost=MAX_SUM;
         char **base=best_base_tours;

         do {
            tf = new FindTour(*base, m, &d, 0, NULL);
            if (param.verbose > 1)
               dump << tf->tour->cost(m) << " " << *base << "\n";
            if (tf->tour->cost(m) < bestcost) {
               bestcost = tf->tour->cost(m);
               tour.copy(*tf->tour);
            }
            delete tf;
         } while (*(++base) != NULL);
         ret = new NOptHeuristic(m, &tour, ROPTH);
/*
         NOptHeuristic nopt(m, &tour, 3);
         while (nopt.run())
            ;
         d += nopt.duration;
         ret = new NOptHeuristic(m, nopt.tour, ROPT_TUPLE);
 */
      }
      else if (m->is_oneway()) {
         FindTour tf("loss+th+fo+k3+th+k14+th", m, &d, 0, NULL);
         ret = new NOptHeuristic(m, tf.tour, 3);
      }
      else {
         FindTour tf("pa+th+fo+k3+th+k14+th", m, &d, 0, NULL);
         ret = new APSolver(m, tf.tour, 14);
      }
      ret->duration += d;
      break;
   case 7: ret = new AdditionHeuristic(m, NORMAL_ADDITION); break;
   case 8: ret = new AdditionHeuristic(m, FARTHEST_ADDITION); break;
   case 9: ret = new DispersionHeuristic(m); break;
   case 10: ret = new AdditionHeuristic(m, ANGLE_ADDITION);
      break;
   case 11: ret = new EasyTourHeuristic(m, 0); break;
   case 26: ret = new EasyTourHeuristic(m, 1); break;
   case 27: ret = new OneTreeSolver(m, 0); break;
   case 28: ret = new APSolver(m, 0); break;
   case 6: ret = new APSolver(m, 1); break;
   case 13:
      {
         FindTour tf(m->is_symmetric() ? "fa" : "ps", m, &d, 0, NULL);
         ret = new NOptHeuristic(m, tf.tour, 3);
         ret->duration += d;
      }
      break;
   case 20: ret = new MultiFragHeuristic(m); break;
   case 21: ret = new BestInOutHeuristic(m); break;
   case 12: ret = new LossHeuristic(m); break;
   case 24: ret = new AdditionHeuristic(m, SPLIT_ADDITION); break;
   case 32: ret = new SavingsHeuristic(m, 0); break;
   case 33: ret = new FlexMapHeuristic(m); break;
   default: ret = NULL; break;
   }
   return ret;
}

TourFinder *switch_tourimprovers(short x, const Matrix *m, Tour *t)
{
   TourFinder *ret;
   switch (x) {
   case 14: ret = new NOptHeuristic(m, t, 3); break;
   case 15: ret = new NOptHeuristic(m, t, 4); break;
   case 16: ret = new NOptHeuristic(m, t, 2); break;
   case 17: ret = new NOptHeuristic(m, t, 5); break;
   case 18: ret = new APSolver(m, t, 4); break;
   case 19: ret = new APSolver(m, t, 14); break;
   case 22: ret = new OneTreeSolver(m, t, 5); break;
   case 23: ret = new OneTreeSolver(m, t, 17); break;
   case 25: ret = new NOptHeuristic(m, t, ROPT_TUPLE); break;
   case 31: ret = new NOptHeuristic(m, t, KSWAP10_TUPLE); break;
   case 26: ret = new EasyTourHeuristic(m, t, 1); break;
   default: ret = NULL; break;
   }
   return ret;
}
