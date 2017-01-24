/* standard.h
 * the standard solver
 * orig ceh
 */

#ifndef _STANDARD_H
#define _STANDARD_H

#include "solver.h"
#include "stanquel.h"
#include "pqueue.h"
#include "params.h"

class Standard : public TSPSolver {
   node_t *current;
   PQueue que;
   city_id_t base_nodes_dequed;
   unsigned long *explores;
public:
   Standard (const Matrix *m);
   ~Standard();

   int run();
   int can_run(const Matrix *m) const;
};

#endif
