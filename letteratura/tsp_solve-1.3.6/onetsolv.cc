/* onetsolv.cc
 *
 * simple branching 1-tree s best solverolver
 *
 * started 12-9-93 ceh
 */

#include "stdmacro.h"
#include "onetsolv.h"
#include "tour.h"
#include "pqueue.h"
#include "findtour.h"
#include <assert.h>
#include "parallel.h"
#include "onetquel.h"

static void st_clip(PQuelem *n)
{
   unuse_matrix(((onode_t*)n)->sm);
}

int OTreeSolver :: can_run(const Matrix *m) const
{
#if defined(LARGE_CITY_ID) || defined(UNSIGNED_CITY_ID)
   if ((long)degree > 150)
      dump << "It is unwise to run the OneTree Algorithm on a TSP" <<
         "with more than 150 cities\n";
#endif
   return m->is_symmetric();
}

OTreeSolver :: ~OTreeSolver()
{
   delete explores;
}

PQuelem *OTreeSolver :: get_best_node()
{
   city_id_t y, high_id = NO_ID;
   onode_t *init_node = NULL;
   sum_t isum, high = (sum_t)0;
/*
   city_id_t num;
   sum_t loopt, wholesum, sum;
   long conns;
*/

   for (y=0; y<degree; y++) {
      onode_t *curr = new onode_t (y, sm, this);
      isum = curr->optimal;
      /*
      sum = wholesum = isum;
      loopt = MAX_SUM;
      num = 0;
      conns = curr->c_factor();
      city_id_t x;
      for (x = curr->depth+1; x < degree; x++) {
         onode_t n(*curr, x, best_so_far);
         if (n.optimal < best_so_far) {
            if (loopt > n.optimal)
               loopt = n.optimal;
            sum += n.optimal;
            num++;
         }
         wholesum += n.optimal;
         conns += curr->c_factor();
      }
      double base;
      base=(double)sum/(num*num);
      if (param.verbose>2) {
         dump << isum << " " << wholesum << " " << num << " "
            << base <<" ["<<y<<"]\n";
      }
       */
      if (param.verbose>2)
         dump << isum << "\n";
      if (high < isum) {
         if (init_node != NULL)
            delete init_node;

         init_node = curr;
         high = isum;
         high_id = y;
      }
      else
         delete curr;
   }
   delete init_node;
   init_node = new onode_t (high_id, sm, this, best_so_far);
   if (param.verbose) {
      dump << "High " << high_id << "\n";
      dump.flush();
   }
   return init_node;
}


/* Point *1*: unfortunatley the sort has to be done first to assign the
 * real_cost values of the sorted matrix.
 * ceh
 */
OTreeSolver :: OTreeSolver(const Matrix *m) : TSPSolver (m)
{
   city_id_t x;
   onode_t *init_node;
   SortedMatrix SM(m->degree);

   install_blocked_mallocs(sizeof(onode_t), 100);

   clipper = st_clip;
   explores = new unsigned long[m->degree-2];
   if (!can_run(m))
      return;
   init_otree_stats();
   SM.sort(m, NO_ID); /*1*/
   sm = start_using_matrix(SM);
   if (param.verbose>10) {
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
   if (param.verbose>1)
      dump << "Best Heuristic " << best_so_far << "\n";

   if (param.initial_choice == NO_ID)
      init_node = (onode_t*)get_best_node();
   else if (param.initial_choice != NO_ID && param.initial_choice < m->degree)
      init_node = new onode_t(param.initial_choice, sm, this, best_so_far);
   else
      init_node = new onode_t(0, sm, this, best_so_far);
   if (param.verbose)
      dump << "Initial Lower Bound " << init_node->optimal << "\n";
   Initial_Lower_Bound = init_node->optimal;
   if (param.verbose>10)
      sm->print();
   que.enq(init_node);
   base_nodes_dequed = m->degree;
   for (x=0; x<(city_id_t)(m->degree-2); x++)
      explores[x] = 0;
   if (que.empty()) {
      if (param.verbose)
         dump << "Nothing IN QUEUE\n";
      unuse_matrix(sm);
   }
}

/* Point *1*: This could destruct the current element if the que.most was
 * less than the newly reborn elements optimal.  So we should break out
 * of the loop right then and not look at current references at all.
 *
 * Point *2*: not only break out of switch but break out of loop as well
 */
int OTreeSolver :: run()
{
   city_id_t x, depth;
   onode_t *current = NULL;

   while (!que.empty()) {
      current = (onode_t*)que.deq();
/*
que.print();
 */
      assert(current->optimal <= que.most);
      /*
      if (sig_caught) {
         sig_caught = 0;
         Term file(0);
         file.open("tspinfo.txt", "w", 0);
         que.print(file);
         free_otree_stats(file);
         tour->print(file);
         file.close();
      }
       */
      if (param.verbose>10) {
         que.print();
         current->print();
      }
      if (current->depth == degree-2) {
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
         explores[depth = current->depth]++;
         current->branch(que);
         if (depth == 1) {
            --base_nodes_dequed;
            break;
         }
      }
   }
   if (que.empty()) {
      percent_left = 0;
      if (param.verbose) {
         double exps=0;
         for (x=0; x<(city_id_t)(degree-2); x++) {
            if (explores[x] != 0)
            dump.form("Level %d was Explored %lu Times.\n",(int)x,explores[x]);
            exps += explores[x];
         }
         dump << "Total Explores " << exps << "\n";
         free_otree_stats(dump);
      }
      if (!tour->is_complete(matrix->degree) && parallel.my_session_done()) {
         dump << "ERR: Upper Bound -oArg was lower than Optimal Tour.  ";
         dump << "Use a higher value.\n";
         assert(param.most != MAX_SUM);
      }
   }
   else {
      percent_left = (short)((100*(long)base_nodes_dequed)/(long)degree);
      percent_left = (percent_left == 0) ? 1 : percent_left;
   }
   return percent_left;
}
