/* assignql.h
 * AP Que element
 * orig ceh 12-10-94
 */
#define ITERS

#ifndef _ASSIGNQL_H
#define _ASSIGNQL_H

#include "stdmacro.h"
#include "pqueue.h"
#include "matrix.h"
#include "assign.h"
#include <assert.h>

#define MAX_ZEROES 50

#ifdef __BORLANDC__
#define inline
#endif

#ifdef ITERS

# ifdef FLOAT_COST
#  define CITY_TO_COST(_city) (-((cost_t)_city)-.5)
#  define COST_TO_CITY(_cost) ((city_id_t)-(_cost))
# else
#  define CITY_TO_COST(_city) (-((cost_t)_city)-1)
#  define COST_TO_CITY(_cost) ((city_id_t)-(_cost)-1)
# endif

typedef cost_t *zero_pointer;

class Zero_Set
{
public:
   // The elements of the set
   cost_t start;
   cost_t *cost;

   inline zero_pointer assert_add(city_id_t i);
   inline void operator = (const Zero_Set &aps);
   Zero_Set();
   inline void init(cost_t *c) { cost = c; };
   inline void init_iter(zero_pointer &i);
   inline int next_iter(zero_pointer &i);
   inline int init_iter(zero_pointer &i, city_id_t &);
   // returns if iter is at last (never finishes iterating)
   inline int next_iter(zero_pointer &i, city_id_t &);
   inline int ok(zero_pointer &i);
   inline int end_iter(zero_pointer &i);
   inline int two_or_more();
   inline int empty();
   inline zero_pointer find(const city_id_t i);
   inline void remove(zero_pointer p);
   inline void remove(const city_id_t i);
   inline void if_exists_remove(const city_id_t i);
   inline void remove_all_except(const city_id_t i, const city_id_t);
   inline void write(BinFile &t) const;
   inline void read(BinFile &t, cost_t *);
};

#else

# define CITY_TO_COST(_city) (_city)
# define COST_TO_CITY(_cost) (_cost)
# define Zero_Set AP_Set
# define zero_pointer czero_pointer

#endif

typedef city_id_t *czero_pointer;

class AP_Set
{
public:
   // The elements of the set
   city_id_t *zsp, zs[MAX_ZEROES];

   inline void add_it(city_id_t i);
   inline czero_pointer assert_add(city_id_t i);
   void operator = (const AP_Set &aps);
   AP_Set();
   void init_iter(czero_pointer &i);
   int next_iter(czero_pointer &i);
   int init_iter(czero_pointer &i, city_id_t &);
   // returns if iter is at last (never finishes iterating)
   int next_iter(czero_pointer &i, city_id_t &);
   int ok(czero_pointer &i);
   int end_iter(czero_pointer &i);
   int two_or_more();
   int empty();
   inline czero_pointer find(const city_id_t i);
   inline void remove(czero_pointer p);
   inline void remove(const city_id_t i);
   inline void if_exists_remove(const city_id_t i);
   inline void remove_all_except(const city_id_t i, const city_id_t);
   void write(BinFile &t) const;
   void read(BinFile &t, cost_t *);
};

class apcity_t {
public:
   city_id_t assmnt, assnor, next, prev;
#ifdef ITERS
   Zero_Set zeroes;
#else
   AP_Set zeroes;
#endif

   // returns true if the city has been forced to go to it's collumn
   inline int is_included() { return prev == NO_ID; };
   void write(BinFile &t) const;
   void read(BinFile &t, cost_t *);
};

class anode_t : public PQuelem
{
   int find_min(AP_Set *, const sum_t);
   void assignment(city_id_t, const sum_t);
   void initial_solution();
   void make_assignment(const city_id_t, const city_id_t);
   void base_construct(const Matrix *m, TSPSolver *s);
   void set_equal(const anode_t &); // a partial = operator
   void impede(const sum_t most, const city_id_t current_id);
   void include(city_id_t, city_id_t);
   void exclude(city_id_t, city_id_t);

   // The node maker to invoke a new node from a currently constructed branch
   anode_t (const anode_t &, const city_id_t,
      const city_id_t, const sum_t most);

   // includes all (a...z) and then excludes final arc after z
   anode_t (const anode_t &, const sum_t, const city_id_t a, const city_id_t z);

   APSolver *solver;
   apcity_t *cities, *mic_end;
   // the included cities are those that are forcibly assigned by other cities
   city_id_t included, unincluded;
public:
   city_id_t depth, degree;
   Matrix *aprime;
   const Matrix *matrix;
   sum_t optimal;

   PQuelem *read_clone(BinFile &, TSPSolver *) const;
   void write(BinFile &) const;

   // Patching operation to build tour from AP
   void construct_tour(Tour *, int) const;

   anode_t (const Matrix *m, TSPSolver *s);

   void branch(PQueue &que);
   void fast_branch(PQueue &que);

   ~anode_t();
   anode_t();

   // The initial node maker to start the branching tour
   anode_t (const Matrix *, TSPSolver *s, const sum_t); 

   // Similer to the new invocation of a child but, destroy's the current
   // object and assigns its "last" child over it, returning the addres to this
   // as the new "reborn" child node, who encompasses the same space the parent
   // did.
   anode_t *rebirth(const city_id_t, const city_id_t, const sum_t most);

   // simple copy constructor
   anode_t (const anode_t &);

   // simple clone (allocate and copy and return pointer to clone)
   PQuelem * clone() const { return new anode_t(*this); };

   // element printing for quelem print
   void printn() const;

   // for k-swaping
   anode_t (const anode_t &parent, const city_id_t sub, const city_id_t k,
      Tour *, const sum_t);
};

#endif
