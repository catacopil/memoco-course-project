/* choice0.cc
 *
 * for a simple heuristic executable
 *
 * orig ceh
 */

#include "disperse.h"
#include "assign.h"
#include "nopt.h"
#include "findtour.h"
#include "easytour.h"
#include "hungaria.h"
#include "flexmap.h"
#include "addition.h"
#include "choice.h"

choice choices[]=
{
   {"heurbest", "Best Heuristic", 0}, //0
   {"best", "Best Optimal", 0}, //1
   {"k3swap", "3-Swap", 0}, //2
   {"assign", "AP B&B", 0}, //3
   {"treeopt", "Asym 3-Opt", 0}, //4
   {"fouropt", "Asym 4-Opt", 0}, //5
   {"dispersion", "Dispersion", 0}, //6
   {"k14swap", "14-Swap", 0}, //7
   {"flexmap", "FlexMap", 0}, //8
   {"addition", "Convex Addition", 0}, //9
   {"split", "Split Addition", 0}, //10
   {"farthest", "Farthest Insertion", 0}, //11
   {"angleadd", "Angle Addition", 0}, //12
   {"easytour", "EasyTour", 0}, //13

   {NULL, NULL, 0}
};

TourFinder *switch_tourfinders(short x, const Matrix *m)
{
   TourFinder *ret;
   switch (x) {
   case 0:
   case 6: ret = new DispersionHeuristic(m); break;
   case 1:
   case 3: ret = new APSolver(m); break;
   case 8: ret = new FlexMapHeuristic(m); break;
   case 9: ret = new AdditionHeuristic(m, NORMAL_ADDITION); break;
   case 10: ret = new AdditionHeuristic(m, SPLIT_ADDITION); break;
   case 11: ret = new AdditionHeuristic(m, FARTHEST_ADDITION); break;
   case 12: ret = new AdditionHeuristic(m, ANGLE_ADDITION); break;
   case 13: ret = new EasyTourHeuristic(m, 0); break;

   default: ret = NULL; break;
   }
   return ret;
}

TourFinder *switch_tourimprovers(short x, const Matrix *m, Tour *t)
{
   TourFinder *ret;
   switch (x) {
   case 7: ret = new APSolver(m, t, 14); break;
   case 2: ret = new NOptHeuristic(m, t, 2); break;
   case 4: ret = new NOptHeuristic(m, t, 3); break;
   case 5: ret = new NOptHeuristic(m, t, 4); break;
   default: ret = NULL; break;
   }
   return ret;
}
