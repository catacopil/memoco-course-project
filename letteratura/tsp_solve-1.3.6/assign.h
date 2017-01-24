/* assign.h
 * the standard solver
 * orig ceh 5-27-94
 */

#ifndef _ASSIGN_H
#define _ASSIGN_H

#include "solver.h"
#include "pqueue.h"

class APSolver : public TSPSolver {
   city_id_t base_nodes_dequed;
   unsigned long *explores;
   int make_kswap_branches();
   sum_t last_best_so_far;
   int what;
   city_id_t ksize, last_head, kswap_increment;
   city_id_t max_mantissa, non_improvements, kextra_size;
   int write(BinFile &, const PQueue &) const;

public:
   void remove_unassignment(const city_id_t);
   void add_unassignment(const city_id_t);
   int split(BinFile &);
   void split_done(int);
   int can_split();
   int read(BinFile &);
   int save(BinFile &) const;
   int save(Term &) const;

   city_id_t unassigned, *rows, *cols, *next, *Unassigned;
   APSolver (const Matrix *m);
   APSolver (const Matrix *m, int); // AP patching
   APSolver (const Matrix *m, Tour *t, city_id_t k);
   ~APSolver();

   int run();
   int can_run(const Matrix *m) const;
};

#endif
