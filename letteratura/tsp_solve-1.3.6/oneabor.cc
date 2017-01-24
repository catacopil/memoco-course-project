/* oneabor.cc
 *
 * Improved branching 1-Aboresence solver
 *
 * started 12-9-93 ceh
 */

/*
#define SIGDEBUG
#define PRINTIT
 */
#define MAX_NON_IMPROVEMENTS (2*degree/ksize)

#include "stdmacro.h"
#include "oneabor.h"
#include "oneaborq.h"
#include "tour.h"
#include "pqueue.h"
#include "findtour.h"
#include <assert.h>
#include "binfile.h"
#include "parallel.h"

#define ONEABOR_UNIQ 0x07e4304L

int OneAborSolver :: can_run(const Matrix *m) const
{
#if defined (LARGE_CITY_ID) || defined(UNSIGNED_CITY_ID)
   if ((unsigned short)degree > (unsigned short)150)
      dump << "It is unwise to run the OneAbor Algorithm on a TSP" <<
         "with more than 150 cities\n";
#endif
   return m!=NULL;
}

OneAborSolver :: ~OneAborSolver()
{
   if (explores != NULL) {
      delete explores;
      explores = NULL;
   }
   if (minarray != NULL) {
      delete minarray;
      minarray = NULL;
   }
   if (xarray != NULL) {
      delete xarray;
      xarray = NULL;
   }
}

OneAborSolver :: OneAborSolver(const Matrix *m, int) : TSPSolver (m)
{
   explores = NULL;
   minarray = NULL;
   xarray = NULL;
   if (!can_run(m))
      return;
   OneAbor *o = new OneAbor(this, tour);
   delete o;
}

OneAborSolver :: OneAborSolver(const Matrix *m) : TSPSolver (m)
{
   city_id_t x;
   OneAbor *init_node;

   ksize = 0;
   install_blocked_mallocs(sizeof(OneAbor), 100);
   install_blocked_mallocs(8/*sizeof(forbidden_city)*/, degree);
   explores = new unsigned long[degree];
   x = 2*degree - 2;
   stack = (line = (shadow = (arcj = (arci = (parent = (label = (xarray
    = new city_id_t[15*degree]) + degree) + x) + x) + x) + x) + x) + x;
   bigm = (minarray = new lambdacost_t[x*x+x]) + x;
   loc2 = bigm + degree;
   init_oneabor_stats();
   catchable = USR1CAUGHT|USR2CAUGHT;
   if (!can_run(m))
      return;
   if (param.recover_bits & RECOVERTOURFINDER) {
      read(*param.bf);
      param.recover_bits &= ~RECOVERTOURFINDER;
dump << "Read OneAbor\n";
      param.try_close();
      return;
   }
   if (param.verbose>2) {
      m->print();
   }
   if (param.most == 0 || degree < 8) {
      que.most = SUM_GRANU;
      for (x=0; x<degree; x++)
         que.most += m->val(x, (x+1)%degree);
      best_so_far = que.most;
   }
   else if (param.most != MAX_SUM) {
      best_so_far = que.most = param.most;
   }
   else {
      FindTour t("he", m, &duration, 0, NULL);
      tour->copy(*t.tour);
      best_so_far = que.most = t.length;
   }
   if (param.verbose>1)
      dump << "Best Heuristic " << best_so_far << "\n";

   if (param.initial_choice != NO_ID && param.initial_choice < degree)
      init_node = new
         OneAbor(param.initial_choice, this, best_so_far);
   else
      init_node = new OneAbor(0, this, best_so_far);
   if (best_so_far < que.most)
      que.most = best_so_far;
   if (param.verbose)
      dump << "Initial Lower Bound " << init_node->optimal << "\n";
   Initial_Lower_Bound = init_node->optimal;
   que.enq(init_node);
   base_nodes_dequed = degree;
   for (x=0; x<degree; x++)
      explores[x] = 0;
   if (que.empty()) {
      if (param.verbose)
         dump << "Nothing IN QUEUE\n";
   }
   update_tour(tour, 1);
}

int OneAborSolver :: save(Term &file) const
{
   que.print(file);
   free_oneabor_stats(file);
   tour->print(file);
   file << best_so_far << "\n";
   return 1;
}

/* Point *1*: This could destruct the current element if the que.most was
 * less than the newly reborn elements optimal.  So we should break out
 * of the loop right then and not look at current references at all.
 *
 * Point *2*: not only break out of switch but break out of loop as well
 * C.eH
 */
int OneAborSolver :: run()
{
   city_id_t x, depth;
   OneAbor *current=NULL;

   if (explores == NULL)
      return 0;
   while (!caught() && !que.empty()) {
      current = (OneAbor*)que.deq();
      assert(current->optimal < que.most);
#ifdef SIGDEBUG
if (param.verbose)
dump << getpid() << " " << current->optimal << " Que\n";
#endif
      if (param.verbose>10) {
         que.print();
         current->print();
      }
      explores[depth = current->depth]++;
      if (depth == degree-1) {
         Tour a_tour(degree);
         current->construct_tour(&a_tour);
         if (a_tour.cost(matrix) + SUM_GRANU < current->optimal)
            dump << " UGLY " << a_tour.cost(matrix) << ","
               << current->optimal << "\n";
         if (a_tour.cost(matrix) < best_so_far) {
            update_tour(&a_tour, 1);
         }
      }
      else {
         /*
         current->sym_branch(que);
          */
         current->branch(que);
         if (depth == 1) {
            --base_nodes_dequed;
            break;
         }
      }
   }
   if (que.empty()) {
      if (!make_kswap_branches()) {
         percent_left = (short)
          ((100*(long)(MAX_NON_IMPROVEMENTS-non_improvements))
           / MAX_NON_IMPROVEMENTS);
         percent_left = (percent_left == 0) ? 1 : percent_left;
         return percent_left;
      }
      percent_left = 0;
      if (param.verbose) {
         double exps=0;
         for (x=0; x<degree; x++) {
            if (explores[x] != 0) {
               dump.form("Level %d was Explored %lu Times.\n",
                  (int)x, explores[x]);
               exps += explores[x];
            }
         }
         dump << "Total Explores " << exps << "\n";
         free_oneabor_stats(dump);
      }
      parallel.update_new_tour(tour, best_so_far, 1, matrix);
      if (!tour->is_complete(matrix->degree) && parallel.my_session_done()) {
         dump << "ERR: Upper Bound -oArg was lower than Optimal Tour.  ";
         dump << "Use a higher value.\n";
         assert(param.most != MAX_SUM);
      }
   }
   else {
      if (base_nodes_dequed <= 0)
         base_nodes_dequed = 1;
      percent_left = (short)((100*(long)base_nodes_dequed)/(long)degree);
      percent_left = (percent_left == 0) ? 1 : percent_left;
   }
   return percent_left;
}

int OneAborSolver :: save(BinFile &file) const
{
   return write(file, que);
}

int OneAborSolver :: write(BinFile &file, const PQueue &q) const
{
   city_id_t d;

   file << ONEABOR_UNIQ;
   save_solver(file);
   file << base_nodes_dequed << base_nodes_left << param.most;
   for (d=0; d<degree; d++)
      file << explores[d];
   q.write(file);
   file << ONEABOR_UNIQ;
   return 1;
}

int OneAborSolver :: read(BinFile &file)
{
   city_id_t d;
   OneAbor onetree;
   long l;
   sum_t j = param.most;

   file >> l;
   die_if(l != ONEABOR_UNIQ, "File Reading Error", "");
   load_solver(file);
   file >> base_nodes_dequed >> base_nodes_left >> param.most;
   if (j < best_so_far)
      best_so_far = param.most = j;
   assert(explores != NULL);
   delete explores;
   delete minarray;
   delete xarray;
   explores = new unsigned long[degree];
   d = 2*degree - 2;
   stack = (line = (shadow = (arcj = (arci = (parent = (label = (xarray
    = new city_id_t[15*degree]) + degree) + d) + d) + d) + d) + d) + d;
   bigm = (minarray = new lambdacost_t[d*d+d]) + d;
   loc2 = bigm + degree;
   for (d = 0; d < degree; d++)
      file >> explores[d];
   que.read(file, onetree, this);
   update_tour(tour, 0);
   file >> l;
   die_if(l != ONEABOR_UNIQ, "File Reading Error", "");
   return 1;
}

int OneAborSolver :: can_split()
{
   if (current_split == NULL && (current_split = que.antideq()) == NULL) {
      dump << "Won't Antideq\n";
      dump.flush();
      return 0;
   }
   return 1;
}

int OneAborSolver :: split(BinFile &file)
{
   OneAbor *current = (OneAbor*)current_split;
   PQueue newq;
   int ok;

   newq.enq(current);
   ok = write(file, newq);
   newq.deq();
   newq.dont_delete_last_dequeued();
   return ok;
}

void OneAborSolver :: split_done(int ok)
{
   OneAbor *current = (OneAbor*)current_split;

   if (current != NULL) {
      if (ok)
         delete current;
      else
         que.enq(current);
      current_split = NULL;
   }
   update_tour(tour, 0);
}

int OneAborSolver :: make_kswap_branches()
{
   if (ksize < 3)
      return 1;
   if (last_best_so_far == best_so_far) {
#ifdef PRINTIT
dump << "Non Improve\n";
#endif
      if (++non_improvements == MAX_NON_IMPROVEMENTS)
         return 1;
   }
   else {
#ifdef PRINTIT
dump << "--Improved "  << last_best_so_far << "->" << best_so_far << "\n";
#endif
      non_improvements = 0;
      last_best_so_far = best_so_far;
   }
   kswap_increment = (kswap_increment + ksize) % degree;
   OneAbor base(this), *cur;
#ifdef PRINTIT
dump << "[" << kswap_increment << "," << ksize << "]\n";
#endif
   que.enq(cur = new OneAbor(base, kswap_increment, ksize, tour));
   if (Initial_Lower_Bound == 0 || Initial_Lower_Bound > cur->optimal)
      Initial_Lower_Bound = cur->optimal;
   return 0;
}

// use prime numbers for k
OneAborSolver :: OneAborSolver(const Matrix *m, Tour *t, city_id_t k)
 : TSPSolver (m)
{
   city_id_t x;

   ksize = 0;
   if (degree-2 <= k)
      k = degree-3;
   last_best_so_far = SUM_MAX;
   kswap_increment = 0;
   install_blocked_mallocs(sizeof(OneAbor), 100);
   install_blocked_mallocs(8/*sizeof(forbidden_city)*/, degree);
   init_oneabor_stats();
   explores = new unsigned long[degree];
   x = 2*degree - 2;
   stack = (line = (shadow = (arcj = (arci = (parent = (label = (xarray
    = new city_id_t[15*degree]) + degree) + x) + x) + x) + x) + x) + x;
   bigm = (minarray = new lambdacost_t[x*x+x]) + x;
   loc2 = bigm + degree;
   for (x=0; x<degree; x++)
      explores[x] = 0;
   base_nodes_dequed = degree;
   if (!can_run(m))
      return;
   delete tour;
   tour = new Tour(degree);
   Path *p;
   TourIter ti(*t);
   for (; (p = ti.next())!=NULL; )
      tour->travel(p->id);
   ksize = k;
   while (degree%ksize==0)
      ksize += 6;
   que.most = best_so_far = tour->cost(m);
   make_kswap_branches();
   if (que.empty()) {
      if (param.verbose)
         dump << "Nothing IN QUEUE\n";
   }
}
