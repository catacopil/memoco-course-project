/* oneacity.h
 *
 * cities in a 1-aboresence
 *
 * orig ceh
 */

#ifndef _ONEACITY_H
#define _ONEACITY_H

/*
#define MAX_FORBIDDEN 10
#define SECORD
 */

#include "stdmacro.h"
#include <assert.h>
#include "binfile.h"
#include "tour.h"
#include "oneabor.h"

// if section is 2 then city is on the tail of the required edge
// if section is 4 then city is on the head of the required edge

class AMatching;

typedef struct {
   // required_edge1 is the tail from, requiried_edge2 is the head to
   city_id_t required_edge1, required_edge2;
} sec1_t;
typedef struct {
   city_id_t end, onetree_edge, required_edge;
} sec2_t;
typedef struct {
   city_id_t onetree_edge;
} sec3_t;
typedef union {
   sec1_t sec1;
   sec2_t sec2;
   sec3_t sec3;
} Ustuff;

class forbidden_cities {
public:
#ifdef MAX_FORBIDDEN
   city_id_t forbidden[MAX_FORBIDDEN];
   inline forbidden_cities() { forbidden[0] = NO_ID; }
#else
   typedef struct forbidden_city {
      struct forbidden_city *next;
      city_id_t id;
   } forbidden_city;
   forbidden_city *forbidden;
   inline forbidden_cities() { forbidden = NULL; }
#endif
   ~forbidden_cities();
   // if edge j coming IN to this city is forbidden
   int not_forbidden(const city_id_t j) const;
   // forbid edge j coming IN to this city
   void forbid(const city_id_t j);
   city_id_t iter(void *&);
   void operator = (const forbidden_cities &);
   void read(BinFile &);
   void write(BinFile &) const;
};

class city_a {
public:
   lambdacost_t lambda;
#ifdef SECORD
   lambdacost_t secord_val;
#endif
   forbidden_cities fs;
   signed_city_id_t branches;
   Ustuff u;
   unsigned char section;

   void operator = (const city_a &);
   inline city_id_t get_id(const city_a *ml) const
      { return (city_id_t)(this-ml); };

   inline int is_section2() const { return !(section % 2); };
   inline void reset() {
      switch (section) {
      case 4: case 2: u.sec2.onetree_edge = NO_ID; branches = 1; break;
      case 3: u.sec3.onetree_edge = NO_ID; branches = 0; break;
      }
   }
   inline ~city_a() {};
   inline city_a () { section = 3; u.sec3.onetree_edge = NO_ID; branches = 0;
      lambda = 0; };
   inline city_a (const city_a &c) { *this = c; };

   // returns the id that is most from, if section = 3 then returns id, if
   // section 2 then the id which is at the tail end of the edge required edge
   // is returned.
   inline city_id_t tail(const city_a *ml) const
      { return (section == 3 || section == 2)
      ? get_id(ml) : (assert(section == 4), u.sec2.end); };
   inline city_id_t head(const city_a *ml) const
      { return (section == 3 || section == 4)
      ? get_id(ml) : (assert(section == 2), u.sec2.end); };
   inline city_id_t up_root(const city_a *ml) const
   {
      switch (section) {
      case 3: return u.sec3.onetree_edge;
      case 2: case 4:
         return ((u.sec2.onetree_edge == NO_ID)
          ? ml[u.sec2.end].up() : u.sec2.onetree_edge);
      case 1: return u.sec1.required_edge1;
      default: assert(0);
      }
      return NO_ID;
   }
   inline city_id_t up() const
      { return (is_section2() ? u.sec2.onetree_edge : u.sec3.onetree_edge); };
   inline city_id_t &upset()
      { return (is_section2() ? u.sec2.onetree_edge : u.sec3.onetree_edge); };
   inline city_id_t &upset(city_a *ml)
      { return (is_section2() ? ml[u.sec2.end].u.sec2.onetree_edge = NO_ID
      , u.sec2.onetree_edge : u.sec3.onetree_edge); };
   inline city_id_t &slow_up()
   {
      return (is_section2() ? ((u.sec2.onetree_edge == NO_ID)
       ? u.sec2.end : u.sec2.onetree_edge) : u.sec3.onetree_edge);
   }
   inline const city_id_t up(const city_id_t last) const
   {
      assert(section == 1);
      if (last == u.sec1.required_edge1)
         return u.sec1.required_edge2;
      else {
         assert(last == u.sec1.required_edge2);
         return u.sec1.required_edge1;
      }
   }
   inline const city_id_t going_up(const city_id_t last) const
   {
      if (section == 1)
         return up(last);
      return (is_section2() ? ((u.sec2.onetree_edge == NO_ID)
       ? ((last == u.sec2.required_edge) ? NO_ID : u.sec2.required_edge)
       : u.sec2.onetree_edge) : u.sec3.onetree_edge);
   }

   void read(BinFile &);
   void write(BinFile &) const;

   void place(AMatching *list, AMatching *& last_clean, city_a *ml);
};

class forbidden_cities_array {
public:
   forbidden_cities *list, *end_city;
   forbidden_cities_array() { list = NULL; }
   void init (const city_id_t j)
   {
      char *c = new char[sizeof(city_a)*j];
      forbidden_cities a, *l;
      for (l = list = (forbidden_cities*)c, end_city = l+j; l < end_city; ) {
#ifndef MAX_FORBIDDEN
         l->forbidden = NULL;
#endif
         *l++ = a;
      }
   }
   ~forbidden_cities_array();
};

class city_a_array
{
public:
   void init (const city_id_t j)
   {
      char *c = new char[sizeof(city_a)*j];
      city_a a, *l;
      for (l = list = (city_a*)c, end_city = l+j; l < end_city; ) {
#ifndef MAX_FORBIDDEN
         l->fs.forbidden = NULL;
#endif
         *l++ = a;
      }
   }
   city_a *list, *end_city;
   city_a_array() { list = NULL; };
   ~city_a_array()
   {
      city_a *l;
      if (list != NULL) {
         for (l = list; l < end_city; l++)
            l->city_a::~city_a();
         delete (char*)list;
      }
   }
   void operator = (const city_a_array &p)
   {
      const city_a *pl = p.list;
      city_a *ml = list;
      while (ml < end_city)
         *ml++ = *pl++;
   }
   void write(BinFile &t) const
   {
      city_a *ml = list;
      for (ml = list; ml < end_city; ml++)
         ml->write(t);
   }
   city_a *operator + (const city_id_t j) { return list+j; };
   const city_a *operator + (const city_id_t j) const { return list+j; };
   city_id_t operator - (const city_a *p) { return (city_id_t)(list-p); };
   city_a &operator [] (const city_id_t j) { return list[j]; };
   const city_a &operator [] (const city_id_t j) const { return list[j]; };
   operator city_a *() { return list; };
   operator const city_a *() const { return list; };
};

#define B1TRAVELED 0x1
#define B2TRAVELED 0x2
#define MATRAVELED 0x4
#define CITRAVELED 0x8
#define UPTRAVELED 0x10 /* means all cities above 2 have been traveled */

class AMatching {
public:
   union {
      cost_t concost;
      city_id_t previous;
   } u;
   city_id_t next, branches, b1, b2, match, traveled;
   inline AMatching() { match = next = NO_ID; branches = traveled = 0; };
   void find_clean_branch(AMatching *list, city_id_t b, city_id_t p,
      AMatching *& last_clean);
   void place_branch(AMatching *list, city_id_t b, AMatching *& last_clean);
   AMatching *find_next_untraveled(AMatching *list);
   void travel(AMatching *list, city_id_t b);
   void print(AMatching *);
   void depth_first(AMatching *list, Tour *, Path *, int pr);
};

#endif
