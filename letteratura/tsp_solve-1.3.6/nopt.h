/* nopt.h
 *
 * for asymmetric TSPs, swap N-tuple edges if tour can be made better.
 *
 * orig ceh 6-1-94
 */
#ifndef _NOPT_H
#define _NOPT_H

#include "tourfind.h"

#define ROPTH 12
#define ROPT_TUPLE 11
#define KSWAP10_TUPLE 10
#define SKSWAP10_TUPLE 9

class NOptHeuristic : public TourFinder {
   void swap_it(city_id_t *, city_id_t *);
   void SWAP4(city_id_t *a, city_id_t *b, city_id_t *c,
      city_id_t *d, int e, sum_t diff);
   city_id_t *tourcopy, *xtracopy, tuple;
   void printr(city_id_t *);
   void k3swap();
   void three_opt();
   void four_opt();
   void five_opt();
   void ropt();
   void kswap(int);
   city_id_t find_j_and_swap(city_id_t *k, const city_id_t i);
   city_id_t find_j(city_id_t *k, const city_id_t i);
   void recurse(int level);
   void swap4(city_id_t *, city_id_t *, city_id_t *, city_id_t *, int);
   void rand_tourcopy();
   long trys;
   cost_t **cost;
public:

   ~NOptHeuristic();
   NOptHeuristic (const Matrix*, Tour *, int Tuple);
   int can_run(const Matrix *) const;
   int run();
};

#endif
