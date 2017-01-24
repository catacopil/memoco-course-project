/* standard.cc
 *
 * standard best solver using least arcs as relaxation.
 *
 * started 11-19-93 ceh
 */

#include "stdmacro.h"
#include "standard.h"
#include "tour.h"
#include "pqueue.h"
#include "findtour.h"

Standard :: ~Standard()
{
   delete explores;
}

int Standard :: can_run(const Matrix *m) const
{
   if (m->degree > 25) {
      dump << "It is Unwise to run this slow of an algorithm on a TSP\n" <<
         "which is more than 25 cities.\n";
   }
   return m->degree <= 25;
}

/* here we very much improve the rest of the running of the standard
 * tsp by choosing our first site to travel very carefully.  This is done
 * by pretending to use as the Initial_ID each for all sites in the
 * graph.  We then tally each of the initial optimal values of each of
 * the pretend start ups and choose the initial id that resulted in the
 * highest optimal bound.  This can improve speed by half in most circumstances.
 *
 * Yes i know half of infinity is still infinity.  But at least on some
 * problems that breach infinity that are run on finite machines, it makes
 * things go by faster.
 *
 * I just hope this same method will work with other relaxations, besides
 * the least arcs relaxation used by the Standard solver.
 */
Standard :: Standard(const Matrix *m) : TSPSolver (m)
{
   sum_t sum, high=0;
   city_id_t y,x, high_id=0;

   install_blocked_mallocs(sizeof(node_t), 100);

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
   if (param.initial_choice == NO_ID) {
      for (y=0; y<degree; y++) {
         node_t current(y, m);
         sum = 0;
         for (x=(city_id_t)(current.depth+1); x<degree; x++) {
            node_t n(current, x, best_so_far);
            sum += n.optimal;
         }
         if (param.verbose)
            dump << " = " << sum << " [" << y << "]\n";
         if (high < sum
          && (!matrix->is_oneway() || y != (city_id_t)(degree-1))) {
            high = sum;
            high_id = y;
         }
      }
   }
   else if (param.initial_choice != NO_ID && param.initial_choice < degree)
      high_id = param.initial_choice;
   else
      high_id = (city_id_t)(degree-1);
   if (param.verbose)
      dump << "High " << high_id << "\n";
   que.enq(new node_t(high_id,m));
   current = NULL;
   base_nodes_dequed = degree;
   explores = new unsigned long[degree-2];
   for (x=0; x<(city_id_t)(degree-2); x++)
      explores[x] = 0;
}

/* Point *1*: This could destruct the current element if the que.most was
 * less than the newly reborn elements optimal.  So we should break out
 * of the loop right then and not look at current references at all.
 */
int Standard :: run()
{
   city_id_t x, depth;

   while (!que.empty()) {
      current = (node_t*)que.deq();
      /*
      que.print();
      current->print();
       */
      if (current->depth == degree-2) {
         if (current->optimal < best_so_far) {
            if (best_so_far != SUM_MAX) {
               delete tour;
               tour = new Tour(degree);
            }
            que.strip(best_so_far = current->optimal, NULL);
            for (x = 0; x < degree; x++) {
               tour->travel(current->mic_list[x].mic_id);
            }
            if (param.verbose) {
               dump << "FOUND " << best_so_far <<" "<< tour->cost(matrix)
                  << "\n";
               tour->print();
            }
         }
      }
      else {
         depth = current->depth;
         explores[depth]++;
         for (x=(city_id_t)(current->depth+1); x<degree; x++) {
            if (current->mic_list[x].impeded == NOT_IMPEDED) {
               que.enq( new node_t (*current, x, que.most) );
            }
            else if (current->mic_list[x].impeded == LAST_NOT_IMPEDED) {
               que.enq( current->rebirth(x, que.most) );  /*1*/
               que.dont_delete_last_dequeued();
               break;
            }
         }
         if (depth == 1) {
            --base_nodes_dequed;
            break;
         }
      }
   }
   if (que.empty()) {
      percent_left = 0;
      if (param.verbose)
         for (x=0; x<(city_id_t)(degree-2); x++)
            dump.form("Level %d was Explored %lu Times.\n", (int)x,explores[x]);
   }
   else {
      percent_left = (short)((100*(long)base_nodes_dequed) / (long)degree);
      percent_left = (percent_left == 0) ? 1 : percent_left;
   }
   return percent_left;
}
