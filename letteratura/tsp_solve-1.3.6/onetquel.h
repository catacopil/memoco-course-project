/* onetquel.h
 *
 * the simplebranching 1-tree queue element
 * orig ceh
 */

#ifndef _ONETQUEL_H
#define _ONETQUEL_H

#include "stdmacro.h"
#include "pquelem.h"
#include "matrix.h"
#include "smatrix.h"
#include "tour.h"
#include "params.h"
#include <assert.h>
#include "pqueue.h"
#include <math.h>
#include "onetsolv.h"

#define MAX_OT_STATS 101

typedef struct {
   long dist[MAX_OT_STATS];
   double Trains, Explores;
} otree_stats;

extern otree_stats st_stats;
extern void init_otree_stats();
extern void free_otree_stats(Term &term);

typedef struct mic_listT_t {
   struct {
      union {
         struct {
            city_id_t m_sindex;          // id of parent in min span tree
/*
#ifdef UNSIGNED_CITY_ID
            _uchar m_branches;
#else
 */
            signed char m_branches;
/*
#endif
 */
         } m;
         struct {
            cost_t c_pathcost;      // cost of traveling path
         } c;
      } uu;
   } u;
//   city_id_t sindex;          // id of parent in min span tree
   city_id_t span_id;         // id the spaning edge is from
   _uchar impeded;
} mic_listT_t;

// if required_edges == 2 then it is not included in the onetree
/*
#define reqruited_edges impeded    // number of required edges
 */
#define sindex u.uu.m.m_sindex
#define pathcost u.uu.c.c_pathcost
#define brnchs u.uu.m.m_branches

class onode_t : public PQuelem
{
   inline void spawn(const onode_t &, const city_id_t, const sum_t);
   void min_span();
   inline void incremental_min_span(city_id_t , const city_id_t);
   void make_root(city_id_t);
   void construct(const onode_t &);
   int sort_lambda_costs(const sum_t optimost);
   inline void sum_lambdas();
   int train_lambdas(sum_t optimost, int max_iters);
   inline void do_impedance(sum_t optimost);
   inline void init_first_constructor (city_id_t, SortedMatrix *, TSPSolver *);
   class OTreeSolver *solver;

public:
   // returns non-zero if the node
   inline int will_train(const sum_t most);
   inline int stat_will_train(const sum_t most);

   void operator=(const onode_t &);

   ~onode_t() { delete mic_list; };

   // The initial node maker to start the branching tour
   onode_t (city_id_t, SortedMatrix *, TSPSolver *, sum_t best_yet); 

   // The initial node maker to start the branching tour with trainig lambdas
   onode_t (city_id_t, SortedMatrix *, TSPSolver *); 

   // The node maker to invoke a new node from a currently constructed branch
   onode_t (const onode_t &, const city_id_t, const sum_t most);

   // Similer to the new inocation of a child but, destroy's the current
   // object and assigns its "last" child over it, returning the addres to this
   // as the new "reborn" child node, who encompasses the same space the parent
   // did.
   onode_t *rebirth(const city_id_t, const sum_t most);

   // simple copy constructor
   onode_t (const onode_t &);

   // constructor with a new matrix
   onode_t (const onode_t &, SortedMatrix *sm);

   // element printing for quelem print
   void print() const;

   void Print() const;

   SortedMatrix *sm;
   const Matrix *matrix;
   SortedCost *initcost;     // one of the special 1-tree edges from init
   sum_t optimal;            // optimum completion value
   city_id_t depth;               // for the depth in the queue

   // for some circumstances the one-tree found can be a tour, rather
   // than solving the one-tree all the way down to the bottom we can
   // assign depth to degree-2, so the sovler will think this is a tour.
   // but if the solve finds out that the pure_depth is not degree-2
   // it will construct the tour with the cities traveled so far then
   // construct the rest of the tour from the minimum spaning tree that
   // is a tour.  The function used to do this is construct_tour.
   city_id_t pure_depth;

   mic_listT_t *mic_list;    // THE mic_lists of the node

   // contributing lambda sum which goes in optimal
   // this is not necessarily the sum of all the lambdas but it is the
   // part of optimal that must be taken out to make optimal the cost of
   // the real cositing one tree.
   lambda_sum_t lambda_sum;

   // this city is the second lowest one tree forced edge from the
   // last city traveled to this city
   city_id_t one_tree_city;

   // returns the sum of each sqr(connection - 2)
   long c_factor();

   // makes a tour from the relaxation  will assert() if relaxation
   // is not a tour.
   void construct_tour(Tour *);

   void branch(PQueue &que);

   void check_connections();
};

#endif
