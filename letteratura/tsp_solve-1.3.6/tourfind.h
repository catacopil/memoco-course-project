/* tourfind.h
 * the base tour finder
 * orig ceh
 */

#ifndef _TOURFIND_H
#define _TOURFIND_H

#include "tour.h"
#include "matrix.h"
#include "params.h"
#include "binfile.h"

#define TIMB_SIZE 12 // can be enlarged if assert() fails
#define USR1CAUGHT 0x1
#define USR2CAUGHT 0x2

extern short signals_;

class Timer
{
   short milliseconds;
   short started;
   long seconds;
   void construct();
   char timb[TIMB_SIZE];
public:
   Timer ();
   ~Timer ();
   Timer (const Timer &);

   void reset(); // reset the timer to zero
   void start(); // start the timer, the first call will start with no time
   void stop(); // stop the timer, and the timer may be started again later
   void operator += (const Timer &);
   inline short was_started() { return started; };

   // if subtraction will yield negative, then current timer is reset
   void operator -= (const Timer &);

   void operator = (const Timer &);

   void read(BinFile &);
   void write(BinFile &) const;

   void print(Term &t = dump) const;
   void print_ms() const;
};

class TourFinder
{
protected:
   city_id_t degree;
   short percent_left, catchable;
   void catchsig();
public:
   const Matrix *matrix;
   Timer duration;
   Tour *tour;

   TourFinder (const Matrix*);
   virtual ~TourFinder();

   // run will return the percentage of the algorithm is yet to be done.
   // the TourFinder will be done when run returns 0 (nothing left to do.)
   // when run() returns 0, get_TourFound() can be called to get the TourFound.
   // run() still can return 0, but the TourFinder might have not found a Tour.
   virtual int run();

   // prints the tour
   void print();

   // returns the length of the tour
   sum_t length();

   // returns boolean if the tourfinder can run on a given matrix
   // some algorithms are only specified for certain types of matricies.
   virtual int can_run(const Matrix*) const;

   // returns non-zero if split is possible.
   virtual int can_split();

   // expects at least one call to can_split() before invoked and it
   // splits the work of this tourfinder continuing on half of the work and
   // saving the other have to be restarted by load().
   // returns if successful.
   virtual int split(BinFile &);

   // to be called after split().  if the file was saved and everything is
   // ok then a nonzero is passed, other wise zero is passed and the split is
   // corrected and replaced as if there was never a split done.
   virtual void split_done(int ok);

   // loads entire tour finder saved by save
   // returns if successful
   virtual int load(BinFile &);

   // saves entire tour finder to be loaded and restarted by read.
   // returns if successful
   virtual int save(BinFile &) const;

   // reads and writes TourFinder specific info in easy readable text format
   virtual int save(Term &) const;

   inline int caught() {
      if (signals_ && (signals_ & catchable))
         catchsig();
      return signals_;
   }
};

#endif
