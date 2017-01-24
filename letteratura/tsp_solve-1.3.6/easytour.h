/* easytour.h
 *
 * an easy tour finder that simply finds the tour in the order the cities are
 * input.  Very usefull for passing tours to other tours by inputing the
 * TSP in the order of the tour you want to pass.  If random is non-zero then
 * the tour will be a random tour seeded by the current time, while care is
 * taken to reseed the generator, not to disturb new matricies from being built.
 *
 * orig ceh
 */
#ifndef _EASYTOUR_H
#define _EASYTOUR_H

#include "tourfind.h"

class EasyTourHeuristic : public TourFinder
{
public:
   EasyTourHeuristic (const Matrix*, int random);
   EasyTourHeuristic (const Matrix*, Tour *, int random);
   int can_run(const Matrix *) const;
   int run();
};

#endif
