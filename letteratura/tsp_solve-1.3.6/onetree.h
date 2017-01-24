/* onetree.cc
 * the 1-tree solver
 * orig ceh
 */

#ifndef _ONETREE_H
#define _ONETREE_H

#include "solver.h"
#include "pqueue.h"
#include "smatrix.h"

class OneTreeSolver : public TSPSolver {
   int write(BinFile &, const PQueue &) const;
   city_id_t base_nodes_dequed, base_nodes_left;
   unsigned long *explores;
   int make_kswap_branches();
   city_id_t ksize, last_head, kswap_increment, non_improvements;
   sum_t last_best_so_far;
   SortedMatrix *base_sm; // only used for kswaping

public:
   sum_t diff_total;
   long diffs;
   int split(BinFile &);
   void split_done(int);
   int can_split();
   int read(BinFile &);
   int save(BinFile &) const;
   int save(Term &) const;
   // use prime numbers for k
   OneTreeSolver(const Matrix *m, int); // Christofides Algorithm
   OneTreeSolver(const Matrix *m, Tour *t, city_id_t k);
   OneTreeSolver(const Matrix *m);
   ~OneTreeSolver();

   int run();
   int can_run(const Matrix *m) const;
};


#endif
