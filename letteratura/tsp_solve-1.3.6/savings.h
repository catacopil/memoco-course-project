/* savings.h
 *
 * implimentation of the savings heuristic (an adaptation from a vehicle
 * routing algorithm.)
 *
 * orig ceh 5-23-95
 */
#ifndef _SAVINGS_H
#define _SAVINGS_H

#include "tourfind.h"

class SavingsHeuristic : public TourFinder
{
public:
   SavingsHeuristic (const Matrix*, int start_nodes);
   int can_run(const Matrix *) const;
   int run();
   void print(Tour **);
};

#endif
