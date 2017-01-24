/* tourfind.cc
 *
 * the tour finder base class
 *
 * orig ceh
 */

#include "tourfind.h"
#include "stdmacro.h"
#include <assert.h>
#include <stdlib.h>
#include "parallel.h"
#include "runer.h"
#include "params.h"

short signals_ = 0;

#ifdef HAS_FTIME
# include <sys/types.h>
# include <sys/timeb.h>
# ifndef HAS_FTIME_DECL
extern "C" {
   extern void ftime(struct timeb *);
}
# endif
#else // !HAS_FTIME
# include <time.h>
extern "C" {
# ifndef HAS_CLOCK_DECL
   extern long clock();
# endif
#ifndef HAS_TIME_DECL
   extern time_t time(time_t*);
#endif
}
typedef struct TimeB {
   time_t time;
# ifdef HAS_CLOCK
   unsigned long clock;
# endif
   _uchar time_only; // if the clock time is not counted this is set
} TimeB;
#endif // HAS_FTIME

TourFinder :: TourFinder (const Matrix *m)
{
   duration.start();
   matrix = m;
   degree = m->degree;
   tour = new Tour(m->degree);
   percent_left = 100;
   catchable = 0;
}

TourFinder :: ~TourFinder()
{
   duration.stop();
   if (tour != NULL)
      delete tour;
}

int TourFinder :: run()
{
   dump << "DUMB RUN\n";
   return 100;
}

void TourFinder :: print()
{
   if (tour != NULL) {
      dump << tour->cost(matrix) << "\n";
      tour->print();
   }
}

sum_t TourFinder :: length()
{
   sum_t s;

   if (tour != NULL)
      s=tour->cost(matrix);
   else
      s=0;
   return s;
}

int TourFinder :: can_run(const Matrix *m) const
{
   dump << "Can't run undefined Tour of " << m->degree << " Matrix\n";
   return 0;
}

void Timer :: reset ()
{
   started = 0;
   seconds = 0;
   milliseconds = 0;
}

void Timer :: construct ()
{
#ifdef HAS_FTIME
   int TIMB_SIZE_too_small = (TIMB_SIZE < sizeof(struct timeb));
#else
   int TIMB_SIZE_too_small = (TIMB_SIZE < sizeof(TimeB));
   TimeB *tb = (TimeB*)timb;
# ifdef HAS_CLOCK
   tb->time_only = 0;
# else
   tb->time_only = 1;
# endif
#endif
   die_if(TIMB_SIZE_too_small, "", "");
}

Timer :: Timer ()
{
   construct();
   reset();
}

void Timer :: operator = (const Timer &t)
{
   started = t.started;
   seconds = t.seconds;
   milliseconds = t.milliseconds;
#ifdef HAS_FTIME
   *(struct timeb*)timb = *(struct timeb *)t.timb;
#else
   *(TimeB*)timb = *(TimeB *)t.timb;
#endif
}

void Timer :: read(BinFile &t)
{
   t >> started >> seconds >> milliseconds;
   if (started) {
      started = 0;
      start();
   }
}

void Timer :: write(BinFile &t) const
{
   Timer tmp(*this);
   t << tmp.started;
   tmp.stop();
   t << tmp.seconds << tmp.milliseconds;
}

Timer :: Timer (const Timer &t)
{
   construct();
   *this = t;
}

Timer :: ~Timer ()
{
}

void Timer :: operator -= (const Timer &T)
{
   short st=started;
   stop();
   Timer tmp(T);
   tmp.stop();
   seconds -= tmp.seconds;
   if ((milliseconds -= tmp.milliseconds) < 0) {
      milliseconds += (short)1000;
      seconds--;
   }
   if (seconds < 0)
      reset();
   if (st)
      start();
}

void Timer :: operator += (const Timer &T)
{
   short st=started;
   stop();
   Timer tmp(T);
   tmp.stop();
   seconds += tmp.seconds;
   if ((milliseconds += tmp.milliseconds) >= 1000) {
      milliseconds -= (short)1000;
      seconds++;
   }
   if (st)
      start();
}

void Timer :: start()
{
   if (started)
      return;
   started = 1;
#ifdef HAS_FTIME
   ftime((struct timeb*)timb);
#else
# ifdef HAS_CLOCK
   ((TimeB*)timb)->clock = clock();
# endif
   ((TimeB*)timb)->time = time(0);
#endif
}

// clock returns a good value of cpu seconds used and is usually %10 slower
// than a real clock if all the cpu cycles are being used by the process.
// Therefor, if the time is greater than the clock maximum 429496 * .90
// then we'll have to determine how many times the clock went around
// by checking how many real seconds have passed.
//
// First we deterine the difference in seconds from the two clocks.
//
// Point *1*: If the now clock difference is close to the max clock and the
// real seconds are less than half the distance to the next wrap around,
// it is proly wise to subtract a wrap, since the clock proly wrapped
// one less time than we thought it did.  And then add the large close
// to max clock difference to the timer.
//
// If TIME_ONLY is defined then the time after a clock() wrap around is only
// counted by the seconds of real time passed and not the clock time.  This
// will be used until the multi-wrap clock() code is completely fixed
//
// Point *2*: this subtraction makes the milliseconds 0 when time_only is set
#define TIME_ONLY
void Timer :: stop()
{
   if (!started)
      return;
   started = 0;
#ifdef HAS_FTIME
   struct timeb now, *tb = (struct timeb*)timb;
   ftime(&now);
   seconds += now.time-tb->time;
   milliseconds += (short)(now.millitm-tb->millitm);
#else // !HAS_FTIME
   TimeB now, *tb = (TimeB*)timb;
   long secs;
   now.time = time(0);
# ifdef HAS_CLOCK
   unsigned long s;
   now.clock = clock();
   if (now.clock < tb->clock)
      s = 4294967295L - (tb->clock - now.clock);
   else
      s = now.clock-tb->clock;
   secs = s/1000000L;
   if (now.time - tb->time > (long)(4294*1.03)) {
#  ifdef TIME_ONLY
      secs = now.time - tb->time;
      s = secs*1000000L - milliseconds*1000L; /*2*/
      tb->time_only = 1;
#  else
      dump << "Many Wraps\n";
      seconds += ((now.time-tb->time)/(long)(4294*1.03)) * 4294;
      if (s > 2147483647L && ((now.time-tb->time)%(long)(4294*1.03)) < 2147) {
         dump << "SubTracted\n"; /*1*/
         seconds -= 4294;
      }
#  endif
   }
   milliseconds += (short)(s-secs*1000000L)/1000;
# else // !HAS_CLOCK
   milliseconds = 0;
   secs = now.time-tb->time;
   tb->time_only = 1;
# endif // HAS_CLOCK
   seconds += secs;
#endif // HAS_FTIME
   if (milliseconds >= 1000) {
      seconds += milliseconds/1000;
      milliseconds %= (short)1000;
   }
   else if (milliseconds < 0) {
      short i = (short)(milliseconds/1000 - 1);
      seconds += i;
      milliseconds -= (short)(i*1000);
   }
   assert(milliseconds < 1000 && milliseconds >= 0);
}

void Timer :: print_ms() const
{
   Timer tmp(*this);
   tmp.stop();
   dump << ((tmp.seconds*1000) + tmp.milliseconds) << "ms";
#ifndef HAS_FTIME
   dump << ( ( ((TimeB*)tmp.timb)->time_only ) ? "T" : "C");
#endif
}

void Timer :: print(Term &t) const
{
   Timer tmp(*this);
   long secs;
   short days;

   tmp.stop();
   days = (short)(tmp.seconds / (3600*24));
   if (days > 0) {
      t << days << " Day" << ((days > 1) ? "s " : " ");
      secs = tmp.seconds - days*(3600*24);
   }
   else
      secs = tmp.seconds;
   t.form("%02d:", (int)(secs/3600));
   secs = (secs % 3600);
   t.form("%02d:", (int)(secs/60));
   secs = (secs % 60);
   t.form("%02d.%02d", (int)(secs), (int)tmp.milliseconds/10);
#ifndef HAS_FTIME
   t << (( ((TimeB*)tmp.timb)->time_only ) ? "T" : "C");
#endif
}

void TourFinder :: catchsig()
{
   char *filename, *wopath;
   int ok = 0, save_bits;

   filename = NULL;
   if (signals_ & USR2CAUGHT) {
      duration.stop();
      if (parallel.could_make_something_to_do() && can_split()) {
         filename = parallel.tmpname(&wopath);
         BinFile bfile(filename, "wb", 0);
         save_bits = param.recover_bits;
         param.recover_bits = RECOVERMATRIX|RECOVERTOURFINDER;
         param.write(bfile);
         param.recover_bits = save_bits;
         matrix->write(bfile);
         split(bfile);
         bfile.close();
         ok = parallel.make_something_to_do(wopath);
         if (!ok)
            parallel.undo_tmpname(filename);
      }
      split_done(ok);
      signals_ &= ~USR2CAUGHT; /*change only when no errno*/
      duration.start();
   }
   if (signals_ & USR1CAUGHT) {
      duration.stop();
      BinFile bfile((param.nfsdir == NULL)
       ? "state000.tsp" : (filename = parallel.tmpname(&wopath)),
       "wb", 0);
      save_bits = param.recover_bits;
      param.recover_bits = ((param.nfsdir == NULL)
       ? RECOVERRUNER : 0)|RECOVERMATRIX|RECOVERTOURFINDER;
      param.write(bfile);
      param.recover_bits = save_bits;
      if (param.nfsdir == NULL)
         runer.write(bfile);
      matrix->write(bfile);
      ok |= save(bfile);
      signals_ &= ~USR1CAUGHT; /*FIX: change only when no errno*/
      bfile.close();
      if (filename != NULL) {
         if (ok)
            parallel.make_new_filename(filename, wopath);
         else
            parallel.undo_tmpname(filename);
      }
      Term file(0);
      file.open("tspinfo.txt", "w", 0);
      save(file);
      file.close();
      duration.start();
      runer.kill_it();
   }
   if (filename != NULL)
      delete filename;
}

int TourFinder :: can_split()
{
   return 0;
}

int TourFinder :: split(BinFile &)
{
   split_done(0);
   return 0;
}

void TourFinder :: split_done(int)
{
   die_if(1, "Must be splitable Tour Finder\n", "");
}

int TourFinder :: save(Term &file) const
{
   tour->print(file);
   return 1;
}

int TourFinder :: save(BinFile &file) const
{
   file << degree << percent_left;
   duration.write(file);
   tour->write(file);
   return 1;
}

int TourFinder :: load(BinFile &file)
{
   file >> degree >> percent_left;
#ifdef OLD_TIMER
   ((Matrix*)matrix)->read(file);
#endif
   duration.read(file);
   tour->read(file);
   return 1;
}
