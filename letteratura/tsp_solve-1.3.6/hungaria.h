/* hungaria.h
 *
 * Hungarian Method
 *
 * modified ceh for c++ tour finder class
 */
#ifndef _HUNGARIA_H
#define _HUNGARIA_H

#include "tourfind.h"

class HungarianSolver : public TourFinder {
   Matrix *wmat;
   cost_t **imatcost;
   city_id_t *fwdptr, *backptr, *major_col, *major_row, *best;
   double count;
   sum_t tweight;
   void explore(city_id_t edges, sum_t cost, city_id_t *row, city_id_t *col);
   sum_t bestedge(city_id_t *row, city_id_t *col,
      city_id_t size, city_id_t **r, city_id_t **c);
   sum_t HungarianSolver :: reduce (city_id_t size,
      city_id_t *row, city_id_t *col, cost_t *rowred, cost_t *colred);
public:
   HungarianSolver(const Matrix*);
   ~HungarianSolver();
   int can_run(const Matrix *) const;
   int run();
};

#endif
