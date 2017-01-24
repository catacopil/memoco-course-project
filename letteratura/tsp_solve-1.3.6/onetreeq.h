/* onetreeq.h
 * the improved branching spanning tree que element
 * orig ceh
 */
#ifndef _ONETREEQ_H
#define _ONETREEQ_H

#include <math.h>
#include <assert.h>
#include "stdmacro.h"
#include "pquelem.h"
#include "matrix.h"
#include "smatrix.h"
#include "tour.h"
#include "params.h"
#include "pqueue.h"
#include "onetcity.h"
#include "onetree.h"
#include "binfile.h"

#define MAX_OT_STATS 101

typedef struct {
   unsigned long dist[MAX_OT_STATS];
   double Trains, Explores;
} onetree_stats;

extern onetree_stats st_ostats;
extern void init_onetree_stats();
extern void free_onetree_stats(Term &term);

/* the maximum number of edges that can be coming from a minimum spanning
 * tree vertex.  Or the maximum number of trees that can be reveal at a
 * subproblem minimum spanning tree "cut".
 */
#define MAX_SPAN_CONNECTIONS (MAX_DEGREE/10+4)

typedef struct {
   city_id_t edges, label;
} tree_t;

typedef struct {
   tree_t trees[MAX_SPAN_CONNECTIONS];
   city_id_t num_trees;
} trees_t;

class OneTree : public PQuelem
{
   city_id_t Label(trees_t *);
   void check_connections();
   inline void spawn(const OneTree &, const city_id_t, const sum_t);
   void min_span();
   void incremental_min_span();
   void make_root(city_id_t);
   void construct(const OneTree &);
   int sort_lambda_costs(const sum_t optimost, int);
   void sum_lambdas();
   int train_lambdas(sum_t optimost, int max_iters);
   inline void init_first_constructor (city_id_t, SortedMatrix *, TSPSolver *);
   smid_t smid;

   // returns non-zero if the node
   inline int will_train(const sum_t most);
   inline int stat_will_train(const sum_t most);

   void cut_edge(const city_id_t a, const city_id_t b);
   void forbid_edge(const city_id_t a, const city_id_t b);
   void require_edge(city_id_t a, city_id_t b);
   void incremental_begin(const int);
   void incremental_end();
   void sacred_span();
   int try_train(const sum_t optimost);

   // Invokes a new one tree subproblem from the Onetree 'o' pased
   // by requiring two edges a <-> b and b <-> c
   OneTree (const OneTree &o, const city_id_t a, const city_id_t b,
      const city_id_t c);

   // Invokes a new one tree subproblem from the Onetree 'o' pased
   // by forbiding edge a <-> b
   OneTree (const OneTree &o, const city_id_t a, const city_id_t b);

   // Inokes a new one tree subproblem from the *this subproblem,
   // using up the sames space, using the city b as the pivot branch,
   // by forbiding a <-> (b->id) and requiring (b->id) <-> c
   OneTree *rebirth(const city_id_t a, const city_id_t b, const city_id_t c);

   short version;
public:
   // the preceding private constructors should be called from branch
   void branch(PQueue &que);

   void operator=(const OneTree &);

   ~OneTree()
   {
      if (mic_list != NULL)
         delete mic_list;
      /*
      if (sm != NULL)
         unuse_matrix(sm);
          */
   };
   OneTree() { mic_list = NULL; };

   // The initial node maker to start the branching tour
   OneTree (city_id_t, SortedMatrix *, TSPSolver *); 

   // The initial node maker to start the branching tour with trainig lambdas
   OneTree (city_id_t, SortedMatrix *, TSPSolver *, sum_t &best_yet);

   // simple copy constructor
   OneTree (const OneTree &);

   // constructor with a new matrix (needed for trainig)
   OneTree (const OneTree &, SortedMatrix *sm);

   // Christofides constructor
   OneTree (SortedMatrix *sm, TSPSolver *, Tour *);

   // element printing for quelem print
   void print() const;

   void trip(const char *str);

   // returns the sum of each sqr(connection - 2)
   long c_factor();

   // makes a tour from the relaxation  will assert() if relaxation
   // is not a tour.
   void construct_tour(Tour *);

   // find a heuristic tour from the onetree
   void heuristic_tour(sum_t &);

   // save and restore
   PQuelem *read_clone(BinFile &, TSPSolver *) const;
   void write(BinFile &) const;

   SortedMatrix *sm;
   OneTreeSolver *solver;
   sum_t optimal;       // optimum completion value
   city_id_t depth;     // number of edges required so far

   city_t *mic_list;    // THE mic_lists of the node also == section1
   city_t *section2;    // section 2 of the onetree cities
   city_t *section3;    // section 3 of the onetree cities
   city_t *end_city;    // the end of the mic_list

   sum_t required_cost; // cost of all the required edges so far

   city_id_t sacred_id, sacred_edge; // the distingushing edge of the one tree

   // contributing lambda sum which goes in optimal
   // this is not necessarily the sum of all the lambdas but it is the
   // part of optimal that must be taken out to make optimal the cost of
   // the real cositing one tree.
   lambda_sum_t lambda_sum;

   // kswaping
   OneTree (const OneTree &parent, const city_id_t sub,
      const city_id_t k, Tour *t);
   OneTree (SortedMatrix *, TSPSolver *); 

   // compute the minimum matching of all odd verticies
   void min_matching(Tour *);
};

#endif
