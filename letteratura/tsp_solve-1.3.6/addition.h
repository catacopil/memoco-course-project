/* addition.h
 *
 * heuristic tourfinder using the addition algorithm
 *
 * orig ceh
 */

#ifndef _ADDITION_H
#define _ADDITION_H

#include "tourfind.h"

#define NORMAL_ADDITION 0x00
#define FARTHEST_ADDITION 0x01
#define ANGLE_ADDITION 0x02
#define SPLIT_ADDITION 0x03
#define TYPEMASK_ADDITION 0x0f

class AdditionHeuristic : public TourFinder {
   void (AdditionHeuristic::*to_add)(city_id_t);
   inline void addition_internal_select(city_id_t &, city_id_t &);
   inline void farthest_internal_select(city_id_t &, city_id_t &);
   cost_t *nearcost;
   int type;
   city_id_t *traveled, *traveled_end, *farcity, cities_traveled, last_traveled;
   city_id_t nearestcity, farthestcity, nearesttourcity;
   void internal_add(const city_id_t, city_id_t &);
   void farthest_add(city_id_t to);
   void addition_add(city_id_t to);
   void angle_add(city_id_t to);
   double dot_product(const pos_t *, const pos_t *, const pos_t *);
public:
   AdditionHeuristic (const Matrix*m, int type);
   virtual ~AdditionHeuristic ();
   int can_run(const Matrix *) const;
   int run();
};

#endif
