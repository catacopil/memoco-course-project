/* pquelem.h
 *
 * a priority queue base element
 *
 * orig ceh
 */
#ifndef _PQUELEM_H
#define _PQUELEM_H

#include "stdmacro.h"
#include "io.h"
#include "binfile.h"

class TSPSolver;

class order_t {
public:
   sum_t sum;
   city_id_t id;
   inline order_t () { };
   void read(BinFile &term);
   void write(BinFile &term) const;
   inline int operator < (const order_t &p)
      { return  id > p.id ? 1 : ((id != p.id) ? 0 : (sum < p.sum) ); };
   inline int operator > (const order_t &p)
      { return  id < p.id ? 1 : ((id != p.id) ? 0 : (sum > p.sum) ); };
   inline void operator = (const order_t &p)
      { sum = p.sum; id = p.id; };
};

class PQuelem {
public:
   order_t ord;
   inline void order(sum_t s, city_id_t i)
   {
      /*
      dump << "NEW DEPTH " <<  s << "," << i << "\n";
       */
      ord.sum = s, ord.id = i;
   };
   PQuelem *next;

   inline PQuelem() { };
   inline PQuelem(const order_t &o) { ord = o; };
   virtual PQuelem *clone () const;
   virtual void print() const;
   virtual void write (BinFile &) const;
   virtual PQuelem *read_clone (BinFile &, TSPSolver *) const;
   virtual ~PQuelem ();
   inline int operator < (const PQuelem &p) { return ord < p.ord; };
   inline int operator > (const PQuelem &p) { return ord > p.ord; };
};

#endif
