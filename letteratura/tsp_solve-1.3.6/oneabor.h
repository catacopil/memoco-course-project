/* oneabor.h
 * the 1-tree solver
 * orig ceh
 */

#ifndef _ONEABOR_H
#define _ONEABOR_H

#include "solver.h"
#include "pqueue.h"

#define MAXLCOST ((lambdacost_t)MAX_SUM*(lambdacost_t)MAX_SUM)
#define MINLCOST ((lambdacost_t)MIN_SUM*(lambdacost_t)MAX_SUM)
typedef float lambdacost_t;
typedef double lambdasum_t;
/*
typedef struct city_g {
   lambdacost_t c;
   city_id_t j, H;
} city_g;
 */

class OneAborSolver : public TSPSolver {
   int write(BinFile &, const PQueue &) const;
   city_id_t base_nodes_dequed, base_nodes_left;
   unsigned long *explores;
   int make_kswap_branches();
   city_id_t ksize, last_head, kswap_increment, non_improvements;
   sum_t last_best_so_far;

public:
   int split(BinFile &);
   void split_done(int);
   int can_split();
   int read(BinFile &);
   int save(BinFile &) const;
   int save(Term &) const;
   city_id_t *xarray, *arcj, *arci, *parent, *label, *stack, *line, *shadow;
   lambdacost_t *minarray, *bigm, *loc2;
   // use prime numbers for k
   OneAborSolver(const Matrix *m, int); // Christofides Algorithm
   OneAborSolver(const Matrix *m, Tour *t, city_id_t k);
   OneAborSolver(const Matrix *m);
   ~OneAborSolver();

   int run();
   int can_run(const Matrix *m) const;
};

#endif
