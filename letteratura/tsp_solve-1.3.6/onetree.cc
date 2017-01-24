/* onetree.cc
 *
 * Improved branching 1-tree solver
 *
 * started 12-9-93 ceh
 */

#define ONETREE_UNIQ 0x07e7433L

/*
#define SIGDEBUG
#define PRINTIT
 */
#define MAX_NON_IMPROVEMENTS (2*degree/ksize)

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "stdmacro.h"
#include "onetree.h"
#include "onetreeq.h"
#include "tour.h"
#include "pqueue.h"
#include "findtour.h"
#include <assert.h>
#include "binfile.h"
#include "parallel.h"

static void st_clip(PQuelem *n)
{
   unuse_matrix(((OneTree*)n)->sm);
}

int OneTreeSolver :: can_run(const Matrix *m) const
{
#if defined(LARGE_CITY_ID) || defined(UNSIGNED_CITY_ID)
   if ((unsigned short)degree > (unsigned short)150 && base_sm != NULL)
      dump << "It is unwise to run the OneTree Algorithm on a TSP" <<
         "with more than 150 cities\n";
#endif
   return m->is_symmetric();
}

OneTreeSolver :: ~OneTreeSolver()
{
   if (base_sm != NULL)
      unuse_matrix(base_sm);
   if (explores != NULL) {
      delete explores;
      free_matrix_stack();
      explores = NULL;
   }
   if (param.verbose == 2) {
      dump << "Average Train Increase Bound : "
       << (diff_total / (diffs ? diffs : 1)) << "\n";
   }
}

OneTreeSolver :: OneTreeSolver(const Matrix *m, int) : TSPSolver (m)
{
   SortedMatrix SM(degree);

   clipper = st_clip;
   base_sm = NULL;
   explores = NULL;
   if (!can_run(m))
      return;
   SM.sort(m, NO_ID);
   OneTree *o = new OneTree(&SM, this, tour);
   delete o;
}

/* Point *1*: unfortunatley the sort has to be done first to assign the
 * real_cost values of the sorted matrix.
 */
OneTreeSolver :: OneTreeSolver(const Matrix *m) : TSPSolver (m)
{
   city_id_t x;
   OneTree *init_node;
   SortedMatrix SM(degree), *sm;

   clipper = st_clip;
   diffs = 0;
   diff_total = 0;
   base_sm = NULL;
   install_blocked_mallocs(sizeof(OneTree), 100);
   init_matrix_stack(degree);
   explores = new unsigned long[degree];
   init_onetree_stats();
   if (!can_run(m))
      return;
   catchable = USR1CAUGHT|USR2CAUGHT;
   if (param.recover_bits & RECOVERTOURFINDER) {
      read(*param.bf);
      param.recover_bits &= ~RECOVERTOURFINDER;
dump << "Read OneTree\n";
      param.try_close();
      return;
   }
   SM.sort(m, NO_ID); /*1*/
   sm = start_using_matrix(SM);
   if (param.verbose > 10) {
      m->print();
      sm->print();
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
   if (param.verbose > 1)
      dump << "Best Heuristic " << best_so_far << "\n";

   if (param.initial_choice != NO_ID && param.initial_choice < degree)
      init_node = new
         OneTree(param.initial_choice, sm, this, best_so_far);
   else
      init_node = new OneTree(0, sm, this, best_so_far);
   if (best_so_far < que.most)
      que.most = best_so_far;
   if (param.verbose)
      dump << "Initial Lower Bound " << init_node->optimal << "\n";
   Initial_Lower_Bound = init_node->optimal;
   if (param.verbose > 10)
      sm->print();
   que.enq(init_node);
   base_nodes_dequed = degree;
   for (x=0; x<degree; x++)
      explores[x] = 0;
   if (que.empty()) {
      if (param.verbose)
         dump << "Nothing IN QUEUE\n";
      unuse_matrix(sm);
   }
   update_tour(tour, 1);
}

int OneTreeSolver :: save(Term &file) const
{
   que.print(file);
   free_onetree_stats(file);
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
int OneTreeSolver :: run()
{
   city_id_t x, depth;
   OneTree *current = NULL;

   if (explores == NULL)
      return 0;
   while (!caught() && !que.empty()) {
      current = (OneTree*)que.deq();
      /*
static save = 1;
      if (save) {
         signals_ |= 0x1;
         save = 0;
      }
       */
#ifdef SIGDEBUG
if (param.verbose)
dump << getpid() << " " << current->optimal << " Que\n";
#endif
      assert(current->optimal < que.most);
      if (param.verbose > 10) {
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
         if (a_tour.cost(matrix) < best_so_far)
            update_tour(&a_tour, 1);
         unuse_matrix(current->sm);
      }
      else {
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
         double exps = 0;
         for (x = 0; x < degree; x++) {
            if (explores[x] != 0) {
               dump.form("Level %d was Explored %lu Times.\n",
                  (int)x, explores[x]);
               exps += explores[x];
            }
         }
         dump << "Total Explores " << exps << "\n";
         free_onetree_stats(dump);
      }
      parallel.update_new_tour(tour, best_so_far, 1, matrix);
      if (!tour->is_complete(matrix->degree) && parallel.my_session_done()) {
         assert(param.most != MAX_SUM);
         dump << "ERR: Upper Bound -oArg was lower than Optimal Tour.  ";
         dump << "Use a higher value.\n";
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

int OneTreeSolver :: save(BinFile &file) const
{
   return write(file, que);
}

int OneTreeSolver :: write(BinFile &file, const PQueue &q) const
{
   city_id_t d;

   file << ONETREE_UNIQ;
   save_solver(file);
   file << base_nodes_dequed << base_nodes_left << param.most;
   for (d = 0; d < degree; d++)
      file << explores[d];
   write_matrix_stack(file);
   q.write(file);
   file << ONETREE_UNIQ;
   return 1;
}

#ifdef OLD_TIMER

int OneTreeSolver :: read(BinFile &file)
{
   city_id_t d;
   OneTree onetree;
   long l;
   sum_t j=param.most;
   file >> l;
   assert(l==ONETREE_UNIQ);
   TourFinder::load(file);
   file >> base_nodes_dequed >> base_nodes_left >> best_so_far
      >> Initial_Lower_Bound >> param.most;
   if (j < best_so_far)
      best_so_far = param.most = j;
   delete explores;
   explores = new unsigned long[degree];
   for (d = 0; d < degree; d++)
      file >> explores[d];
   read_matrix_stack(file, matrix);
   que.read(file, onetree, this);
   file >> l;
   assert(l==ONETREE_UNIQ);
   file.close();
   update_tour(tour, 0);
   dump << "Read Done\n";
   dump.flush();
   duration.start();
   return 1;
}

#else

int OneTreeSolver :: read(BinFile &file)
{
   city_id_t d;
   OneTree onetree;
   long l;
   sum_t j = param.most;

   file >> l;
   die_if(l != ONETREE_UNIQ, "File Reading Error", "");
   load_solver(file);
   file >> base_nodes_dequed >> base_nodes_left >> param.most;
   if (j < best_so_far)
      best_so_far = param.most = j;
   delete explores;
   explores = new unsigned long[degree];
   for (d = 0; d < degree; d++)
      file >> explores[d];
   read_matrix_stack(file, matrix);
   que.read(file, onetree, this);
   update_tour(tour, 0);
   file >> l;
   die_if(l != ONETREE_UNIQ, "File Reading Error", "");
   return 1;
}

#endif

int OneTreeSolver :: can_split()
{
   if (current_split == NULL && (current_split = que.antideq()) == NULL) {
      dump << "Won't Antideq\n";
      dump.flush();
      return 0;
   }
   return 1;
}

int OneTreeSolver :: split(BinFile &file)
{
   OneTree *current = (OneTree*)current_split;
   PQueue newq;
   SortedMatrix *sm;
   int ok;

   assert(current != NULL);
   sm = current->sm;
   init_matrix_stack(degree);
   current->sm = start_using_matrix(*sm);
   newq.enq(current);
   ok = write(file, newq);
   newq.deq();
   newq.dont_delete_last_dequeued();
   unuse_matrix(current->sm);
   free_matrix_stack();
   current->sm = sm;
   return ok;
}

void OneTreeSolver :: split_done(int ok)
{
   OneTree *current = (OneTree*)current_split;

   if (current != NULL) {
      if (ok) {
         clipper(current);
         delete current;
      }
      else
         que.enq(current);
      current_split = NULL;
   }
   update_tour(tour, 0);
}

int OneTreeSolver :: make_kswap_branches()
{
   if (ksize < 3 || base_sm == NULL)
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
   OneTree base(base_sm, this), *cur;
#ifdef PRINTIT
dump << "[" << kswap_increment << "," << ksize << "]\n";
#endif
   use_matrix(base_sm);
   if (!que.enq(cur = new OneTree(base, kswap_increment, ksize, tour)))
      unuse_matrix(base_sm);
   else if (Initial_Lower_Bound == 0 || Initial_Lower_Bound > cur->optimal)
      Initial_Lower_Bound = cur->optimal;
   return 0;
}

// use prime numbers for k
OneTreeSolver :: OneTreeSolver(const Matrix *m, Tour *t, city_id_t k)
 : TSPSolver (m)
{
   Path *p;
   TourIter ti;
   city_id_t x;
   SortedMatrix SM(degree);

   clipper = st_clip;
   if (degree-2 <= k)
      k = degree-3;
   last_best_so_far = SUM_MAX;
   kswap_increment = 0;
   install_blocked_mallocs(sizeof(OneTree), 100);
   init_matrix_stack(degree);
   init_onetree_stats();
   explores = new unsigned long[degree];
   for (x=0; x<degree; x++)
      explores[x] = 0;
   base_nodes_dequed = degree;
   if (!can_run(m))
      return;
   SM.sort(m, NO_ID);
   base_sm = start_using_matrix(SM);
   delete tour;
   tour = new Tour(degree);
   for (ti.init(*t); (p = ti.next()) != NULL; )
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
