/* onetsolv.h
 * the simple branching 1-tree solver
 * orig ceh
 */

#ifndef _ONETSOLV_H
#define _ONETSOLV_H

#include "solver.h"
#include "pqueue.h"
#include "smatrix.h"

class OTreeSolver : public TSPSolver {
   city_id_t base_nodes_dequed;
   unsigned long *explores;
   SortedMatrix *sm;
   PQuelem *get_best_node();
public:
   OTreeSolver (const Matrix *m);
   ~OTreeSolver();

   int run();
   int can_run(const Matrix *m) const;
};

#endif
