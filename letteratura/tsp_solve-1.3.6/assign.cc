/* assign.cc
 *
 * AP Solver
 *
 * started C version 11-19-92 ceh
 *
 * converted to c++ tourfinder 6-2-94
 */

#define ASSIGN_UNIQ 0x00a55167L
/*
#define PRINTIT
#define SIGDEBUG
#define PRINTIT_1
 */

#include "stdmacro.h"
#include "tour.h"
#include "pqueue.h"
#include "parallel.h"
#include "findtour.h"
#include "assign.h"
#include "assignql.h"

int APSolver :: can_run(const Matrix *) const
{
   city_id_t size = (ksize!=0) ? ksize : degree;
   if (size > 175) {
      dump << "It is Unwise to run this slow of an algorithm on a TSP\n" <<
         "which is more than 175 cities.\n";
   }
   return size <= 175;
}

APSolver :: ~APSolver()
{
   delete explores;
   delete rows;
}

APSolver :: APSolver(const Matrix *m) : TSPSolver (m)
{
   city_id_t x;

   /*
   if (param.verbose > 2)
      matrix->print();
       */
   ksize = 0;
   what = 0;
   unassigned = last_head = NO_ID;
   explores = new unsigned long[degree];
   for (x=0; x<degree; x++)
      explores[x] = 0;
   // Must malloc rows, cols, next in that order
   Unassigned = (next = (cols = (rows = new city_id_t[degree*4])
      + degree) + degree) + degree;
#ifndef NDEBUG
   for (x = 0; x < degree; x++)
      Unassigned[x] = NO_ID;
#endif
   catchable = USR1CAUGHT|USR2CAUGHT;
   if (!can_run(m))
      return;
   if (param.recover_bits & RECOVERTOURFINDER) {
      read(*param.bf);
      param.recover_bits &= ~RECOVERTOURFINDER;
dump << "Read Assign\n";
      param.try_close();
      return;
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
   if (param.verbose)
      dump << "Best = " << best_so_far << "\n";
   base_nodes_dequed = degree;
   que.enq(new anode_t(m, this, que.most));
   update_tour(tour, 1);
}

int APSolver :: save(Term &file) const
{
   que.print(file);
   tour->print(file);
   file << best_so_far << "\n";
   return 1;
}

APSolver :: APSolver(const Matrix *m, int sort) : TSPSolver (m)
{
   city_id_t x;

   ksize = 0;
   what = 1;
   unassigned = last_head = NO_ID;
   explores = new unsigned long[degree];
   for (x=0; x<degree; x++)
      explores[x] = 0;
   // Must malloc rows, cols, next in that order
   Unassigned = (next = (cols = (rows = new city_id_t[degree*4])
      + degree) + degree) + degree;
#ifndef NDEBUG
   for (x = 0; x < degree; x++)
      Unassigned[x] = NO_ID;
#endif
   base_nodes_dequed = 0;
   anode_t a(m, this, que.most);
   a.construct_tour(tour, sort);
   /*
city_id_t i;
for (i=0; i<degree; i++)
   tour->travel(i);
    */
}

APSolver :: APSolver(const Matrix *m, Tour *t, city_id_t k) : TSPSolver (m)
{
   city_id_t x;

   what = 0;
   if (degree-2 <= k)
      k = degree-3;
   max_mantissa = (city_id_t)(degree/(k+1));
   kextra_size = (city_id_t)((degree%(k+1))-1);
   last_best_so_far = SUM_MAX;
   kswap_increment = 0;
   install_blocked_mallocs(sizeof(anode_t), 100);
   explores = new unsigned long[degree];
   for (x=0; x<degree; x++)
      explores[x] = 0;
   unassigned = NO_ID;
   // Must malloc rows, cols, next in that order
   Unassigned = (next = (cols = (rows = new city_id_t[degree*4])
      + degree) + degree) + degree;
#ifndef NDEBUG
   for (x = 0; x < degree; x++)
      Unassigned[x] = NO_ID;
#endif
   base_nodes_dequed = degree;
   tour->copy(*t);
   last_head = tour->get_head()->id;
   ksize = k;
   que.most = best_so_far = tour->cost(m);
   make_kswap_branches();
}

/* Point *1*: This could destruct the current element if the que.most was
 * less than the newly reborn elements optimal.  So we should break out
 * of the loop right then and not look at current references at all ceh
 */
int APSolver :: run()
{
   city_id_t x, depth;
   anode_t *current;

   while (!que.empty()) {
      /*
      if (usr1_caught) {
         Term file(0);
         file.open("tspinfo.txt", "w", 0);
         que.print(file);
         tour->print(file);
         file.close();
         write("state000.tsp", que);
         usr1_caught = 0;
      }
      if (usr2_caught) {
         usr2_caught = 0;
         write_split();
         if (que.empty()) {
            dump << "Wue Empty Right Now!\n";
            break;
         }
      }
       */
      current = (anode_t*)que.deq();
#ifdef SIGDEBUG
if (param.verbose)
dump << getpid() << " " << current->optimal << " Que\n";
#endif
      assert(current->optimal < que.most);
      if (param.verbose>10) {
         que.print();
         current->print();
      }
      explores[depth = current->depth]++;
      if (depth == degree-2) {
         Tour a_tour(degree);
         current->construct_tour(&a_tour, 0);
         if (a_tour.cost(matrix) + SUM_GRANU < current->optimal)
            dump << " UGLY " << a_tour.cost(matrix) << ","
               << current->optimal << "\n";
         if (a_tour.cost(matrix) < best_so_far) {
            update_tour(&a_tour, 1);
            /*
            if (last_head!=NO_ID)
               tour->change_head_to(last_head);
                */
         }
      }
      else {
         current->fast_branch(que);
//         current->branch(que);
         if (depth == 1) {
            --base_nodes_dequed;
            break;
         }
      }
   }
   if (que.empty()) {
      if (!make_kswap_branches())
         return percent_left = (short)((100*(long)(ksize+3-non_improvements))
            / (long)(ksize+3));
      percent_left = 0;
      if (param.verbose) {
         double exps=0;
         for (x=0; x<(city_id_t)(degree-2); x++) {
            if (explores[x]) {
               dump.form("Level %d was Explored %lu Times.\n",
                  (int)x, explores[x]);
               exps += explores[x];
            }
         }
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
      percent_left = (short)((100*(long)base_nodes_dequed) / (long)degree);
   }
   return percent_left;
}

void APSolver :: add_unassignment(const city_id_t i)
{
#ifdef PRINTIT_1
dump << "ADDU " << i << "\n";
#endif
   assert(Unassigned[i] == NO_ID);
   Unassigned[i] = unassigned;
   unassigned = i;
}

void APSolver :: remove_unassignment(const city_id_t r)
{
#ifdef PRINTIT_1
dump << "REMU " << r << "\n";
#endif
   assert(unassigned != NO_ID);
   if (unassigned == r)
      unassigned = Unassigned[r];
   else {
      city_id_t *l = Unassigned+unassigned;
      while (*l != r)
         l = Unassigned + *l;
      *l = Unassigned[r];
   }
#ifndef NDEBUG
   Unassigned[r] = NO_ID;
#endif
}

int APSolver :: save(BinFile &file) const
{
   return write(file, que);
}

int APSolver :: write(BinFile &file, const PQueue &q) const
{
   city_id_t d;

   file << ASSIGN_UNIQ;
   save_solver(file);
   file << base_nodes_dequed << param.most;
   for (d = 0; d < degree; d++)
      file << explores[d];
   q.write(file);
   file << ASSIGN_UNIQ;
   return 1;
}

int APSolver :: read(BinFile &file)
{
   city_id_t d;
   anode_t ap;
   long l;
   sum_t j = param.most;

   file >> l;
   die_if(l != ASSIGN_UNIQ, "File Reading Error", "");
   load_solver(file);
   file >> base_nodes_dequed >> param.most;
   if (j < best_so_far)
      best_so_far = param.most = j;
   delete explores;
   explores = new unsigned long[degree];
   for (d = 0; d < degree; d++)
      file >> explores[d];
   que.read(file, ap, this);
   update_tour(tour, 0);
   file >> l;
   die_if(l != ASSIGN_UNIQ, "File Reading Error", "");
   return 1;
}

int APSolver :: can_split()
{
   if (current_split == NULL && (current_split = que.antideq()) == NULL) {
      dump << "Won't Antideq\n";
      dump.flush();
      return 0;
   }
   return 1;
}

int APSolver :: split(BinFile &file)
{
   anode_t *current = (anode_t*)current_split;
   PQueue newq;
   int ok;

   newq.enq(current);
   ok = write(file, newq);
   newq.deq();
   newq.dont_delete_last_dequeued();
   return ok;
}

void APSolver :: split_done(int ok)
{
   anode_t *current = (anode_t*)current_split;

   if (current != NULL) {
      if (ok)
         delete current;
      else
         que.enq(current);
      current_split = NULL;
   }
   update_tour(tour, 0);
}

int APSolver :: make_kswap_branches()
{
   return 1;
/*
   city_id_t inc, mant;

   if (ksize < 3)
      return 1;
   if (last_best_so_far == best_so_far) {
#ifdef PRINTIT
dump << "Non Improve\n";
#endif
      if (++non_improvements == ksize+((kextra_size>2)?1:(kextra_size+1)))
         return 1;
   }
   else {
#ifdef PRINTIT
dump << "--Improved "  << last_best_so_far << "->" << best_so_far << "\n";
#endif
      non_improvements = 0;
      last_best_so_far = best_so_far;
   }
   if (++kswap_increment == degree)
      kswap_increment = 0;
   anode_t base(matrix, this);
   for (inc=kswap_increment, mant=0; mant < max_mantissa; mant++) {
#ifdef PRINTIT
dump << "[" << inc << "," << ksize << "]";
#endif
      que.enq(new anode_t(base, inc, ksize, tour));
      if (inc >= degree-ksize-1)
         inc -= degree-ksize-1;
      else
         inc += ksize+1;
   }
   if (kextra_size > 2) {
#ifdef PRINTIT
dump << "[" << inc << "," << ksize << "]";
#endif
      que.enq(new anode_t(base, inc, kextra_size, tour));
   }
#ifdef PRINTIT
dump << "\n";
#endif
*/
}
