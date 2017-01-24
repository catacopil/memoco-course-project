/* runer.h
 *
 * Class that runs heuristics and keeps track of tour totals.
 *
 * orig ceh 12-25-93
 */
#ifndef _RUNER_H
#define _RUNER_H

#include "stdmacro.h"
#include "matrix.h"
#include "tourfind.h"
#include "binfile.h"

class CostCounter {
   Timer t, time_saved; // time saved is the time that was saved by using prevs
   double sum, sqr_sum;
   long times, max_times, trys;
   CostCounter *prev_start_tour;
public:
   const char *abrev;
   inline int not_done() const { return (trys < max_times); }
   void init();
   void read(BinFile &);
   void write(BinFile &) const;
   Tour last_tour; // last solution found by this CostCounter
   sum_t last_length; // last length found, if SUM_MAX then haven't found yet
   CostCounter(const char *, long mtimes);
   ~CostCounter();
   void operator += (const CostCounter &);
   void count(const Matrix *);
   void print();
};

class Runer {
   CostCounter **the_CostCounters;
   short counters;
   void construct();
public:
   void run(const Matrix *);

   int killed;
   void kill_it(void);

   // compares the first two tour finders and returns non-zero if diff
   int optimal_compare(const Matrix *);

   // just prints out a summary of time taken by the algorithms found
   void show();

   // init_CostCounters must be called after the Param has been initialized
   // with all the tour finder abreviations
   void init();
   Runer();
   ~Runer();
   void operator += (const Runer &);

   // returns if there are still things to run
   int not_done() const;

   // saves the runer as is
   void write(BinFile &) const;

   // reads in the runner as is
   void read(BinFile &);

   // not const because it clears after save
   void save(BinFile &);

   // reads in the runner appending what is loaded to what is in this
   void load(BinFile &);
};

extern Runer runer;

#endif
