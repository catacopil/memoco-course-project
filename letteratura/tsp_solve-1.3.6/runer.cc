/* runer.cc
 *
 * orig ceh 12-25-93
 */

#include "runer.h"
#include "findtour.h"
#include "parallel.h"
#include <assert.h>
#ifdef HAS_SIGNAL
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
#endif
#undef RETSIGTYPE
#define RETSIGTYPE void

#define RUNER_UNIQ 0x43734L

Runer runer;

Runer :: Runer()
{
   killed = 0;
}

Runer :: ~Runer()
{
   if (the_CostCounters != NULL) {
      short a;
      for (a = 0; a < counters; a++)
         delete the_CostCounters[a];
      delete the_CostCounters;
      the_CostCounters = NULL;
   }
}

#ifdef HAS_SIGNAL
static RETSIGTYPE catch_kill(int i)
{
   signal(SIGHUP, SIG_IGN);
   signal(SIGINT, SIG_IGN);
   signal(SIGABRT, SIG_IGN);
   signal(SIGTERM, SIG_IGN);
   runer.killed = i;
   kill(getpid(), SIGUSR1);
}
static RETSIGTYPE catch_abort(int i)
{
   dump << "Aborted at SEED -s" << param.seed-1 << ", Sig #" << i
      << ".  So far the Timers show:\n";
   runer.show();
   parallel.destruct(0);
   exit(1);
}
#endif

void Runer :: kill_it()
{
   if (killed) {
		dump << "Cleanly Killed\n";
      signal(SIGHUP, catch_abort);
      signal(SIGINT, catch_abort);
      signal(SIGABRT, catch_abort);
      signal(SIGTERM, catch_abort);
      kill(getpid(), killed);
   }
}

void Runer :: init()
{
#ifdef HAS_SIGNAL
#ifdef OLD_SIGNAL
   signal(SIGHUP, catch_kill);
   signal(SIGINT, catch_kill);
   signal(SIGABRT, catch_abort);
   signal(SIGTERM, catch_kill);
   signal(SIGSEGV, catch_abort);
   signal(SIGBUS, catch_abort);
   signal(SIGFPE, catch_abort);
#else
   struct sigaction j;
   j.sa_handler = catch_abort;
   j.sa_flags = 0;
   sigfillset(&j.sa_mask);
   if (sigaction(SIGHUP, &j, NULL) == -1
    || sigaction(SIGINT, &j, NULL) == -1 || sigaction(SIGABRT, &j, NULL) == -1
    || sigaction(SIGTERM, &j, NULL) == -1 || sigaction(SIGSEGV, &j, NULL) == -1
    || sigaction(SIGBUS, &j, NULL) == -1 || sigaction(SIGQUIT, &j, NULL) == -1
    || sigaction(SIGFPE, &j, NULL) == -1)
      assert(0);
#endif
#endif
   if (param.recover_bits & RECOVERRUNER) {
      param.recover_bits &= ~RECOVERRUNER;
dump << "Read Runerr\n";
      read(*param.bf);
      param.try_close();
   }
   else {
      this->Runer::~Runer();
      counters = param.num_abrevs;
      construct();
   }
}

void Runer :: construct()
{
   short a;

   the_CostCounters = new CostCounter*[counters];
   for (a = 0; a < counters; a++)
      the_CostCounters[a] = new CostCounter(param.abrevs[a], param.times);
}

void Runer :: run(const Matrix *m)
{
   short a;

   for (a = 0; a < counters; a++) {
      the_CostCounters[a]->count(m);
   }
}


void Runer :: operator += (const Runer &r)
{
   short a, b;

   for (b = 0; b < counters; b++) {
      for (a = 0; a < counters; a++) {
         if (!strcmp(the_CostCounters[a]->abrev, r.the_CostCounters[b]->abrev))
            *the_CostCounters[a] += *r.the_CostCounters[b];
      }
   }
}

// reads in the runner as is
void Runer :: read(BinFile &bf)
{
   short a;
   long r;

   bf >> r;
   die_if(r != RUNER_UNIQ, "File Reading Error", "");
   this->Runer::~Runer();
   bf >> counters;
   construct();
   for (a = 0; a < counters; a++)
      the_CostCounters[a]->read(bf);
   bf >> r;
   die_if(r != RUNER_UNIQ, "File Reading Error", "");
}

// not const because it clears after save
void Runer :: save(BinFile &bf)
{
   short a;

   write(bf);
   for (a = 0; a < counters; a++)
      the_CostCounters[a]->init();
}

// reads in the runner appending what is loaded to what is in this
void Runer :: load(BinFile &bf)
{
   Runer r;
   r.read(bf);
   *this += r;
}

void Runer :: write(BinFile &bf) const
{
   short a;
   long r = RUNER_UNIQ;

   bf << r << counters;
   for (a = 0; a < counters; a++)
      the_CostCounters[a]->write(bf);
   bf << r;
}

int Runer :: not_done() const
{
   short a, ndone;

   for (a = ndone = 0; a < counters; a++) {
      if (the_CostCounters[a]->not_done()) {
         ndone = 1;
         break;
      }
   }
   return ndone;
}

int Runer :: optimal_compare(const Matrix *m)
{
   short a;

   for (a = 0; a < counters; a++) {
      the_CostCounters[a]->count(m);
   }
   return (counters >= 2) ? the_CostCounters[0]->last_tour.cost(m)
    != the_CostCounters[1]->last_tour.cost(m) : 0;
}

void Runer :: show()
{
   short a;
   for (a = 0; a < counters; a++)
      the_CostCounters[a]->print();
}

CostCounter :: ~CostCounter ()
{
}

void CostCounter :: init()
{
   sum = sqr_sum = 0;
   trys = times = 0;
}

CostCounter :: CostCounter(const char *a, long mtimes)
{
   init();
   abrev = a;
   max_times = mtimes;
}

void CostCounter :: write(BinFile &bf) const
{
   bf << abrev << sum << sqr_sum << trys << times << max_times << last_length;
   last_tour.write(bf);
}

void CostCounter :: read(BinFile &bf)
{
   char *name = NULL;
   short a;

   bf >> name >> sum >> sqr_sum >> trys >> times >> max_times >> last_length;
   last_tour.read(bf);
   abrev = NULL;
   for (a = 0; a < param.num_abrevs; a++) {
      if (!strcmp(name, param.abrevs[a]))
         abrev = param.abrevs[a];
   }
   if (name != NULL)
      delete name;
}

void CostCounter :: operator += (const CostCounter &c)
{
   sum += c.sum;
   sqr_sum += c.sqr_sum;
   times += c.times;
   trys += c.trys;
   last_tour.copy(c.last_tour);
   last_length = c.last_length;
}

void CostCounter :: print ()
{
   if (times != 0) {
      dump.form("%-11.11s%5ld run%c ", abrev, times, times==1?' ':'s');
      t.print();
      if (times > 1) {
         double av = sum/times;
         dump << " av=" << av;
         dump << " sd=" << sqrt(sqr_sum/times - av*av);
      }
      else
         dump << " for " << sum;
      dump << "\n";
   }
}

void CostCounter :: count(const Matrix *matrix)
{
   if (not_done()) {
      FindTour ft(abrev, matrix, &t, 1, NULL);
      trys++;
      if ((last_length = ft.length) != SUM_MAX) {
         times++;
         last_tour.copy(*ft.tour);
         sum += ft.length;
         sqr_sum += ft.length*ft.length;
      }
   }
}

