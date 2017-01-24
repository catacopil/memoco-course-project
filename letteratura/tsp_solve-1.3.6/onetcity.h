/* onetcity.h
 *
 * cities in a onetree
 *
 * orig ceh
 */

#ifndef _ONETCITY_H
#define _ONETCITY_H

#include "stdmacro.h"
#include "smatrix.h"
#include <assert.h>
#include "binfile.h"
#include "tour.h"

class Matching;

typedef struct {
   // required_edge1 is the tail from, requiried_edge2 is the head to
   city_id_t required_edge1, required_edge2;
} sec1_t;
typedef struct {
   city_id_t required_edge, onetree_edge, end;
} sec2_t;
typedef struct {
   city_id_t onetree_edge;
} sec3_t;
typedef union {
   sec1_t sec1;
   sec2_t sec2;
   sec3_t sec3;
} Ustuff;

class city_t {
public:
   city_id_t id;
   Ustuff u;
   signed char branches;
   unsigned char section;

   inline void reset() {
      switch (section) {
      case 2: u.sec2.onetree_edge = NO_ID; branches = 1; break;
      case 3: u.sec3.onetree_edge = NO_ID; branches = 0; break;
      }
   };
   inline city_t () { section = 3; u.sec3.onetree_edge = NO_ID; branches = 0; };
   inline city_t (const city_t &c) { *this = c; };

   inline city_id_t &up()
      { return (section == 2 ? u.sec2.onetree_edge : u.sec3.onetree_edge); };
   inline city_id_t &slow_up()
   {
      return (section == 2 ? ((u.sec2.onetree_edge == NO_ID)
       ? u.sec2.end : u.sec2.onetree_edge) : u.sec3.onetree_edge);
   };
   inline city_id_t &up(const city_id_t last)
   {
      assert(section == 1);
      if (last == u.sec1.required_edge1)
         return u.sec1.required_edge2;
      else {
         assert(last == u.sec1.required_edge2);
         return u.sec1.required_edge1;
      }
   };
   city_id_t tree_id() const;

   // same as up except for if it's section2 and NO_ID it returns the end.up()
   // and it returns a unique labeling tree_id of the node or partial tour.
   city_id_t travel_up(const city_t **micmap) const;

   void degrade_section(city_t *(& cptr), const city_id_t new_required_edge);
   void degrade_section(city_t *(& cptr), const city_id_t new_required_edge,
      const city_id_t end);

   lambda_cost_t find_cost(city_id_t i, const SortedMatrix *sm) const;
   /*
   inline lambda_cost_t find_cost(city_id_t i, const SortedMatrix *sm) const
   {
      city_id_t i2;
      for (i2=0; i2<sm->degree; i2++) {
         if (i == sm->cost[id][i2].id)
            break;
      }
      assert(i2 != sm->degree);
      return sm->cost[id][i2].cost;
   };
    */

   void read(BinFile &);
   void write(BinFile &) const;

   void place(Matching *list, Matching *& last_clean);
};

#define B1TRAVELED 0x1
#define B2TRAVELED 0x2
#define MATRAVELED 0x4
#define CITRAVELED 0x8
#define UPTRAVELED 0x10 /* means all cities above 2 have been traveled */

class Matching {
public:
   union {
      cost_t concost;
      city_id_t previous;
   } u;
   city_id_t next, branches, b1, b2, match, traveled;
   inline Matching() { match = next = NO_ID; branches = traveled = 0; };
   void find_clean_branch(Matching *list, city_id_t b, city_id_t p,
      Matching *& last_clean);
   void place_branch(Matching *list, city_id_t b, Matching *& last_clean);
   Matching *find_next_untraveled(Matching *list);
   void travel(Matching *list, city_id_t b);
   void print(Matching *);
   void depth_first(Matching *list, Tour *, Path *, int pr);
};

#endif
