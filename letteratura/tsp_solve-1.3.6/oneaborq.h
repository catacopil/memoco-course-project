/* oneaborq.h
 * the improved branching spanning tree que element
 * orig ceh
 */
#ifndef _ONEABORQ_H
#define _ONEABORQ_H

#include <math.h>
#include <assert.h>
#include "stdmacro.h"
#include "pquelem.h"
#include "matrix.h"
#include "tour.h"
#include "params.h"
#include "pqueue.h"
#include "oneabor.h"
#include "binfile.h"
#include "oneacity.h"

#define MAX_OT_STATS 101

typedef struct {
   unsigned long dist[MAX_OT_STATS];
   double Trains, Explores;
} oneabor_stats;

extern oneabor_stats st_astats;
extern void init_oneabor_stats();
extern void free_oneabor_stats(Term &term);

/*
#define inline
 */

class OneAbor : public PQuelem
{
   int gc;
   void check_connections();
   inline void find_mins(const city_id_t x, const city_id_t t,
      const city_id_t v, const city_id_t m, const city_id_t n,
      const city_id_t bsize, const city_id_t *stkp,
      const city_id_t *stackp, lambdacost_t *bigm);
   inline void spawn(const OneAbor &, const city_id_t, const sum_t);
   void min_span();
   void incremental_min_span();
   void make_root(city_id_t);
   void construct(const OneAbor &);
   int sort_lambda_costs(const sum_t optimost);
   void sum_lambdas();
   int train_lambdas(sum_t optimost, int max_iters);
   inline void init_first_constructor (city_id_t, TSPSolver *);
   inline void new_minarray(const city_id_t x, const city_id_t start,
      const city_id_t xend, const city_id_t *startp, city_id_t *iarray,
      const city_id_t *xarray, lambdacost_t *minarray,
      const forbidden_cities_array &fcs);

   // returns non-zero if the node
   inline int will_train(const sum_t most);
   inline int stat_will_train(const sum_t most);

   void forbid_edge(const city_id_t a, const city_id_t b);
   void require_edge(city_id_t a, city_id_t b);
   void incremental_begin(const int);
   void incremental_end();
   void sacred_span();
   int try_train(const sum_t optimost);

   // Invokes a new one tree subproblem from the Onetree 'o' pased
   // if parm == 1,  by requiring two edges a <-> b and b <-> c
   // if parm == 0,  by requiring a <-> b and forbidding b <-> c
   OneAbor (const OneAbor &o, const city_id_t a, const city_id_t b,
      const city_id_t c, const city_id_t d, int parm);

   // Invokes a new one tree subproblem from the Onetree 'o' pased
   // if parm == 1, by forbiding edge a <-> b
   // if parm == 0, by requiring edge a <-> b
   OneAbor (const OneAbor &o, const city_id_t a, const city_id_t b, int parm);

   // Inokes a new one tree subproblem from the *this subproblem,
   // using up the sames space, using the city b as the pivot branch,
   // by requiring a <-> b and forbidding c <-> d
   OneAbor *rebirth(const city_id_t a, const city_id_t b, const city_id_t c,
      const city_id_t d);

   // requires edge a <-> b
   OneAbor *rebirth(const city_id_t a, const city_id_t b);

public:
   // the preceding private constructors should be called from branch
   void branch(PQueue &que);

   void operator=(const OneAbor &);

   ~OneAbor();
   inline OneAbor() {}

   // The initial node maker to start the branching tour
   OneAbor (city_id_t, TSPSolver *); 

   // The initial node maker to start the branching tour with trainig lambdas
   OneAbor (city_id_t, TSPSolver *, sum_t &best_yet);

   // simple copy constructor
   OneAbor (const OneAbor &);

   // constructor with a new matrix (needed for trainig)
   OneAbor (const OneAbor &, int);

   // Christofides constructor
   OneAbor (TSPSolver *, Tour *);

   // element printing for quelem print
   void print() const;

   void trip(const char *str);

   // returns the sum of each sqr(connection - 2)
   long c_factor();

   // makes a tour from the relaxation  will assert() if relaxation
   // is not a tour.
   int construct_tour(Tour *);

   // find a heuristic tour from the onetree
   void heuristic_tour(Tour *, sum_t &);

   // save and restore
   PQuelem *read_clone(BinFile &, TSPSolver *) const;
   void write(BinFile &) const;

   OneAborSolver *solver;
   sum_t optimal;       // optimum completion value
   city_id_t depth;     // number of edges required so far

   city_a_array mic_list;    // THE mic_lists of the node also == section1

   sum_t required_cost; // cost of all the required edges so far

   city_id_t sacred_id, sacred_edge; // the distingushing edge of the one tree
#ifdef SECORD
   // sacred_secord is the second lowest cost higher than the sacred_edge from
   // the sacred_id.
   lambdacost_t sacred_secord;
#endif

   // contributing lambda sum which goes in optimal
   // this is not necessarily the sum of all the lambdas but it is the
   // part of optimal that must be taken out to make optimal the cost of
   // the real cositing one tree.
   lambdasum_t lambda_sum;

   // kswaping
   OneAbor (const OneAbor &parent, const city_id_t sub,
      const city_id_t k, Tour *t);
   OneAbor (TSPSolver *); 

   // compute the minimum matching of all odd verticies
   void min_matching(Tour *);

   void plot_tree(Term &file=dump);
   void plot_object(pos_t &, pos_t *, int, Term &file=dump);
   void compute_secords();
   inline void check_minarray(lambdacost_t &minarray,
      city_a *city, const city_a *scity, const city_id_t y, const city_id_t x,
      cost_t *cost, city_id_t &onetree_edge, city_id_t &sonetree_edge);
   inline void change_colors_of_children(
      lambdacost_t &, city_id_t &, const city_id_t, const city_id_t);
   inline int same_colors_as_children(const city_id_t, const city_id_t *);
   inline void best_branches(const city_id_t i, const city_id_t a_up,
      lambdacost_t, lambdacost_t &branch1, lambdacost_t &branch2,
      city_id_t &firsta, city_id_t &firstb, city_id_t &secondb, city_id_t &);
   inline lambdacost_t find_best_branches(const city_a *i, city_id_t &firsta,
      city_id_t &firstb, city_id_t &secondb, city_id_t &seconda);
   inline lambdacost_t find_total_secord(const city_a *i, const city_id_t);
   sum_t sym_branch_measure(
      const city_id_t firstb, const city_id_t firsta,
      const city_id_t seconda, const city_id_t secondb,
      OneAbor *&node_one, OneAbor *&node_two, OneAbor *&node_three);
   void sym_best_branch(city_a *city, sum_t &best,
      OneAbor *&node_one, OneAbor *&node_two, OneAbor *&node_three);

   void sym_branch(PQueue &que);
};

#endif
