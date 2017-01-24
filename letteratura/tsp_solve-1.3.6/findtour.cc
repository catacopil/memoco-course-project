/* findtour.cc
 *
 * orig ceh
 */

#include "choice.h"
#include "findtour.h"
#include "io.h"
#include <assert.h>
#include <time.h>
#include "params.h"
#include "chlib.h"
#include "chnew.h"
#include "easytour.h"
#include "runer.h"
#include "parallel.h"
#include "rand.h"
#ifdef HAS_SIGNAL
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>

# ifdef SUNN
#  ifdef SIG_ERR
#   undef SIG_ERR
#  endif
#  define SIG_ERR (void (*)(...))-1
# endif
#endif

#define PLUS_SIGN '+'
#define USR1CAUGHT 0x1
#define USR2CAUGHT 0x2

static void show_tsplib(const TourFinder *, const Matrix *, const choice *);

static short find_tourfinder_index(const _uchar *abrev)
{
   short x, len, ambiguous=0;
   choice *c;

   for (len=0; abrev[len]!=0 && abrev[len]!=PLUS_SIGN; len++)
      ;
   for (c = choices; c->name != NULL; c++)
      c->temp = 0;
   for (c = choices; c->name != NULL; c++) {
      if (!strncmp((char*)abrev, c->abrev, len)) {
         c->temp = 1;
         ambiguous++;
      }
   }
   switch (ambiguous) {
   default:
      dump << "Ambiguous TourFinder!\n'" << abrev
         << "' Could Be one of the possible Tour Finders\n";
      for (c = choices; c->name != NULL; c++) {
         if (c->temp)
            dump << "'" << c->abrev << "' == " << c->name << "\n";
      }
      x = -1;
      break;
   case 0:
      dump << "Can't Choose a TourFinder '" << abrev
         << "'\nThese are the choices:\n";
      for (c = choices; c->name != NULL; c++)
         dump << "'" << c->abrev << "' == " << c->name << "\n";
      x = -1;
      break;
   case 1:
      for (x = 0, c = choices; c->name != NULL; c++, x++) {
         if (c->temp)
            break;
      }
      assert(c->name != NULL);
   }
   return x;
}

static Tour *improve_tour(_uchar *abrev, const Matrix *m, Tour *t, short &x)
{
   TourFinder *tf;
   short y;

   assert(t != NULL);
   for (;;) {
      while (*abrev != PLUS_SIGN && *abrev != 0)
         abrev++;
      if (!(*abrev != 0))
         break;
      y = find_tourfinder_index(++abrev);
      if ((tf = switch_tourimprovers(y, m, t)) == NULL || !tf->can_run(m)) {
         if (tf != NULL)
            delete tf;
         break;
      }
      x = y;
      delete t;
      while (tf->run())
         ;
      t = tf->tour;
      tf->tour = NULL;
      delete tf;
   }
   return t;
}

FindTour :: FindTour(const char *abrev, const Matrix *m,
   Timer *add_timer, int invoked, Tour *to_improve)
{
   TourFinder *st_tf = NULL;
   Stack_Timer s_t(add_timer);
   int i = 1, save_invoked = param.invoked;
   short x;

   length = SUM_MAX;
   tour = NULL;
   param.invoked = invoked;
   if (to_improve == NULL) {
      x = find_tourfinder_index((const _uchar*)abrev);
      st_tf = switch_tourfinders(x, m);
      if (st_tf == NULL) {
         s_t.quit(NULL);
         param.invoked = save_invoked;
         return;
      }
      st_tf->duration.stop();
      if (st_tf->can_run(m)) {
         st_tf->duration.start();
         while ((i = st_tf->run()) != 0) {
            if (param.verbose) {
               dump << i << "%\n";
               dump.flush();
            }
         }
      }
      else {
         dump << "Could Not Run Tour Finder : " << choices[x].name << "\n";
      }
   }
   else {
      st_tf = new EasyTourHeuristic(m, to_improve, 0);
      while (st_tf->run())
         ;
   }
   st_tf->duration.start();
   st_tf->tour = improve_tour((_uchar*)abrev, m, st_tf->tour, x);
   st_tf->duration.stop();
   s_t.quit(st_tf);
   tour = st_tf->tour;
   if (!tour->is_complete(m->degree) && param.nfsdir == NULL) {
      dump << "Tour is NOT Complete from " << choices[x].name << ", SEED "
         << (param.seed-1) << "\n";
      tour->print();
      invoked = 0;
   }
   else
      length = st_tf->length();
   if (invoked) {
      if (param.printing == 2 || param.printing == 3 || param.printing == 1) {
         dump.form("%20.20s", choices[x].name) << " Takes = ";
         st_tf->duration.print(); dump << " for " << length << "\n";
         if (param.printing == 3) {
            if (m->is_oneway())
               tour->print_oneway(dump);
            else
               tour->print();
         }
         dump.flush();
      }
      switch (param.printing) {
      case 1: tour->show(m); break;
      case 4: show_tsplib(st_tf, m, choices+x); break;
      }
   }
   param.invoked = save_invoked;
   st_tf->tour = NULL;
   delete st_tf;
}

FindTour :: ~FindTour()
{
   if (tour != NULL)
      delete tour;
}

/*
#define SPLICE_TOUR
 */

static void show_tsplib(const TourFinder *st_tf, const Matrix *m,
   const choice *c)
{
   Matrix matrix(*m);
   city_id_t *ids = new city_id_t[m->degree], x, y;
   Path *p;
   TourIter ti;

   for (x=0; x<m->degree; x++)
      ids[x] = x;
#ifdef SPLICE_TOUR
   cost_t *co, *ct;
   city_id_t r;

   srand((unsigned)time(0));
   for (x=0; x<m->degree; x++) {
      r = (rand() % (m->degree-x));
      y = ids[x];
      ids[x] = ids[x+r];
      ids[x+r] = y;
   }
   for (x=0; x<m->degree; x++) {
      if (m->is_geometric_2d())
         matrix.pos[ids[x]] = m->pos[x];
      co = matrix.cost[ids[x]];
      ct = m->cost[x];
      for (y=0; y<m->degree; y++)
         co[ids[y]] = ct[y];
   }
#endif

   /*
   if (m->is_generated()) {
    */
      dumpout << "NAME: ???" << m->degree << "\nTYPE: TSP\nCOMMENT: ???\n"
         << "DIMENSION: " << m->degree << "\nEDGE_WEIGHT_TYPE: "
         << find_dname_from_dist(m->dist_funct)->name;
      if (m->is_geometric_2d()) {
         dumpout << "\nNODE_COORD_SECTION\n";
         for (x=0; x<m->degree; x++)
            dumpout.form("%d %.10f %.10f\n", x+1,
               matrix.pos[x].x, matrix.pos[x].y);
      }
      else {
         dumpout << "\nEDGE_WEIGHT_FORMAT: FULL_MATRIX\nEDGE_WEIGHT_SECTION:\n";
         for (y=0; y<m->degree; y++) {
            for (x=0; x<m->degree; x++) {
               if (x==y)
                  dumpout << "0 ";
               else
                  dumpout << m->val(y, x) << " ";
            }
            dumpout << "\n";
         }
      }
      dumpout << "EOF\n";
   /*
   }
    */

   dumpout << "NAME: ???" << m->degree
      << ".opt.tour\nCOMMENT: tsp_solve " << c->name << ", cost = "
      << st_tf->tour->cost(m) << ", "; st_tf->duration.print(dumpout); 
      dumpout << "\nTYPE: TOUR\nDIMENSION: " << m->degree << "\nTOUR_SECTION\n";
   for (ti.init(*st_tf->tour); (p = ti.next()) != NULL; ) {
      dumpout.form("%d\n", (int)(ids[p->id]+1) );
   }
   dumpout << "-1\nEOF\n";

   delete ids;
   return;
}

#ifdef HAS_SIGNAL
static Timer *timer_ptr = NULL;
static RETSIGTYPE catch_cont(int i);

static RETSIGTYPE catch_stop(int i)
{
   runer.show();
   parallel.suspend_process();
#ifdef OLD_SIGNAL
   if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
      dump << i << "ERROR TSTP\n";
#else
   struct sigaction j, k;
   j.sa_handler = SIG_DFL;
   k.sa_handler = catch_cont;
   k.sa_flags = j.sa_flags = 0;
   sigfillset(&j.sa_mask);
   sigfillset(&k.sa_mask);
   if (sigaction(SIGTSTP, &j, NULL) == -1
    || sigaction(SIGCONT, &k, NULL) == -1)
      dump << i << "ERROR STOPS\n";
   else
      dump << "STOPS WORKED\n";
#endif
   if (timer_ptr != NULL)
      timer_ptr->start();
#ifdef OLD_SIGNAL
   if (signal(SIGCONT, catch_cont) == SIG_ERR)
      dump << i << "ERROR CONT\n";
   kill(getpid(), SIGSTOP);
#endif
#ifdef NONVOIDRETSIGTYPE
   return (RETSIGTYPE)i;
#endif
}

static RETSIGTYPE catch_cont(int i)
{
#ifdef OLD_SIGNAL
   if (signal(SIGTSTP, catch_stop) == SIG_ERR)
      dump << i << "ERROR TSTP\n";
   if (signal(SIGCONT, SIG_DFL) == SIG_ERR)
      dump << i << "ERROR CONT\n";
#else
   struct sigaction j, k;
   j.sa_handler = catch_stop;
   k.sa_handler = SIG_DFL;
   k.sa_flags = j.sa_flags = 0;
   sigfillset(&j.sa_mask);
   sigfillset(&k.sa_mask);
   if (sigaction(SIGTSTP, &j, NULL) == -1
    || sigaction(SIGCONT, &k, NULL) == -1)
      dump << i << "ERROR STOPS\n";
   else
      dump << "STOP WORKED\n";
#endif
   parallel.continue_process();
   param.print();
   dump << " Continuing after Suspended Time " << timer_ptr << " ";
   if (timer_ptr != NULL) {
      timer_ptr->stop();
      timer_ptr->print();
   }
   dump << "\n";
   dump.flush();
#ifdef NONVOIDRETSIGTYPE
   return (RETSIGTYPE)i;
#endif
}

static RETSIGTYPE catch_usr2(int i)
{
   signals_ |= USR2CAUGHT;
dump << i << ": Caught 2\n";
#ifdef OLD_SIGNAL
   if (signal(SIGUSR2, catch_usr2) == SIG_ERR)
      dump << i << "ERROR USR2\n";
#endif
#ifdef NONVOIDRETSIGTYPE
   return (RETSIGTYPE)i;
#endif
}

static RETSIGTYPE catch_usr1(int i)
{
   signals_ |= USR1CAUGHT;
dump << i << ": Caught 1\n";
#ifdef OLD_SIGNAL
   if (signal(SIGUSR1, catch_usr1) == SIG_ERR)
      dump << i << "ERROR USR1\n";
#endif
#ifdef NONVOIDRETSIGTYPE
   return (RETSIGTYPE)i;
#endif
}
#endif

Stack_Timer :: Stack_Timer(Timer *add)
{
   quited = 0;
   add_timer = add;
   started = add_timer->was_started();
   add_timer->stop();
#ifdef HAS_SIGNAL
   top_of_stack = (timer_ptr == NULL);
   save_ptr = timer_ptr;
   timer_ptr = &suspend_timer;
#ifdef OLD_SIGNAL
   if (signal(SIGTSTP, catch_stop) == SIG_ERR)
      dump << "ERROR TSTP\n";
   if (signal(SIGCONT, SIG_DFL) == SIG_ERR)
      dump << "ERROR CONT\n";
   if (signal(SIGUSR1, catch_usr1) == SIG_ERR)
      dump << "ERROR USR1\n";
   if (signal(SIGUSR2, catch_usr2) == SIG_ERR)
      dump << "ERROR USR2\n";
#else
   struct sigaction j, k;
   j.sa_handler = catch_stop;
   k.sa_handler = SIG_DFL;
   k.sa_flags = j.sa_flags = 0;
   sigfillset(&j.sa_mask);
   sigfillset(&k.sa_mask);
   if (sigaction(SIGTSTP, &j, NULL) == -1
    || sigaction(SIGCONT, &k, NULL) == -1)
      dump << "ERROR STOPS\n";
   else
      dump << "STOP WORKED\n";
   j.sa_handler = catch_usr1;
   k.sa_handler = catch_usr2;
   k.sa_flags = j.sa_flags = 0;
   sigfillset(&j.sa_mask);
   sigfillset(&k.sa_mask);
   if (sigaction(SIGUSR1, &j, NULL) == -1
    || sigaction(SIGUSR2, &k, NULL) == -1)
      dump << "ERROR USRs\n";
#endif
#endif
}

void Stack_Timer :: quit(TourFinder *tf)
{
   quited = 1;
#ifdef HAS_SIGNAL
   if (tf != NULL)
      tf->duration -= suspend_timer;
   timer_ptr = save_ptr;
   if (timer_ptr == NULL) {
#ifdef OLD_SIGNAL
      if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
         dump << "ERROR TSTP\n";
      if (signal(SIGCONT, SIG_DFL) == SIG_ERR)
         dump << "ERROR CONT\n";
#else
      struct sigaction j;
      j.sa_handler = SIG_DFL;
      j.sa_flags = 0;
      sigfillset(&j.sa_mask);
      if (sigaction(SIGTSTP, &j, NULL) == -1
       || sigaction(SIGCONT, &j, NULL) == -1)
         dump << "ERROR STOPS\n";
      else
         dump << "STOP WORKED\n";
#endif
   }
#endif
   if (tf != NULL)
      *add_timer += tf->duration;
}

Stack_Timer :: ~Stack_Timer()
{
   assert(quited == 1);
   if (started)
      add_timer->start();
}
