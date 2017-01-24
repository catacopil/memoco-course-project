/* stanquel.cc
 *
 * The que element for the standard solver
 * orig ceh
 */

#include "stanquel.h"
#include "io.h"
#include <assert.h>

/* make a mic_list entry be a minimum.
 *
 * list is the entry you wish to change, m is the matrix, mic_list is the
 * begining of the mic_list that you have to cycle through, mic_end is the
 * end of the mic_list.
 */
static void get_minimum_in_cost(register mic_list1_t *list,
 const Matrix *m, register mic_list1_t *mic_list, register mic_list1_t *mic_end)
{
   city_id_t lowest_id         = mic_list->mic_id;
   register cost_t lowest = m->val(lowest_id, list->mic_id);
   register cost_t save_cost;

   for (; ++mic_list < mic_end; ) {
      save_cost = m->val(mic_list->mic_id, list->mic_id);
      if (lowest > save_cost) {
         lowest = save_cost;
         lowest_id = mic_list->mic_id;
      }
   }
   list->orig = lowest_id;
   list->cost = lowest;
}

void node_t :: print() const
{
   city_id_t x;
   dump.form("D%-3d O%-6d ", (int)depth, (int)optimal);
   for (x=0; x<=depth; x++)
      dump.form("T%d[%d(%d)],", (int)mic_list[x].mic_id,
         mic_list[x].cost, (int)mic_list[x].orig);
   for (; x<matrix->degree; x++)
      dump.form("%d[%d(%d)],", (int)mic_list[x].mic_id,
         (int)mic_list[x].cost, (int)mic_list[x].orig);
   dump << "\n";
}

/* Point *2*: we star the get minimum at mic_list+1 because we do not allow
 * any minimums to come from the Initial id who's mic_list is always the 0th
 * this is because we will only come from the 0th city on the first branch
 * of the tree and if we leave a mic_list coming from 0 untouched, it may
 * end up as a minimum in cost there at the end.
 */
node_t :: node_t (city_id_t init_id, const Matrix *m)
{
   city_id_t x;
   mic_list1_t *ml;

   matrix = m;
   install_blocked_mallocs(m->degree*sizeof(mic_list1_t), 100);
   ml = mic_list = new mic_list1_t[m->degree];
   optimal = 0;
   ml->mic_id = init_id;

   for(x=0; x<m->degree; x++) {
      if (x != init_id) {
         ml++;
         ml->mic_id = x;
      }
      ml->cost = COST_MAX;
      ml->impeded = NOT_IMPEDED;
   }

   for (x=0; x<m->degree; x++) {
      /* it does need to search for the inital city for this
      get_minimum_in_cost(mic_list+x, m, mic_list-1,
         mic_list+m->degree);
       */
      get_minimum_in_cost(mic_list+x, m, mic_list,
         mic_list+m->degree);
      optimal += mic_list[x].cost;
   }
   depth = 0;
   order(optimal, depth);
}

/* Point *1*: start the minimum search at depth+1 because the frist
 * node will not need to come from the depth node
 *
 * Point *2*: assign the id_orig as the id that is being covered up and
 * traveled from.  This id is the id that can not be traveled from any
 * of the other elements.
 *
 * Points *3*: are the two places where each mic list is checked to see
 * if they point to a mic_id being covered up, there are two different
 * comparisons because the first node only checks the depth node since
 * that is the one being covered up for the origional node.  And the
 * second place checks the depth-1 (id_orig) since that is being covered
 * up in the point of view of the rest of the mic_lists.
 *
 * Point *4*: Don't even let the node constructed get added on to the queue
 * if the node_t constructed does not have any children of its own
 */
inline void node_t :: spawn(const node_t &parent, const city_id_t sub,
   const sum_t most)
{
   mic_list1_t *ml, *pml, *id_ml = parent.mic_list+sub;
   register city_id_t x, id = id_ml->mic_id, id_orig;

   depth = (city_id_t)(parent.depth+1);
   optimal = parent.optimal;

   ml = mic_list + depth;
   pml = mic_list + (city_id_t)(id_ml-parent.mic_list);
   optimal -= pml->cost;
   *pml = *ml;
   ml->mic_id = id;
   id_orig = ml->orig = (ml-1)->mic_id; /*2*/
   optimal += (ml->cost = matrix->val(ml->orig, ml->mic_id));

   if (mic_list->orig == id) { /*3*/
      optimal -= mic_list->cost;
      get_minimum_in_cost(mic_list, matrix, mic_list+depth+1,  /*1*/
         mic_list+matrix->degree);
      optimal += mic_list->cost;
   }
   for (x=(city_id_t)(depth+1); x<matrix->degree; x++) {
      ml = mic_list+x;
      if (id_orig == ml->orig) { /*3*/
         optimal -= ml->cost;
         get_minimum_in_cost(ml, matrix, mic_list+depth,
            mic_list+matrix->degree);
         optimal += ml->cost;
      }
   }
   int die=1;
   for (x=(city_id_t)(depth+1); x<matrix->degree; x++) {
      ml = mic_list+x;
      if ((ml->impeded = this->impeded(x, most))==NOT_IMPEDED) {
         id_ml = ml;
         die=0;
      }
   }
   if (die) /*4*/
      optimal = MAX_SUM;
   else
      id_ml->impeded = LAST_NOT_IMPEDED;
   order(optimal, depth);
}

node_t :: node_t (const node_t &parent, const city_id_t sub, const sum_t most)
{
   matrix = parent.matrix;
   mic_list = new mic_list1_t[matrix->degree];
   memcpy(mic_list, parent.mic_list, sizeof(mic_list1_t)*matrix->degree);

   spawn(parent, sub, most);
}

node_t * node_t :: rebirth(const city_id_t sub, const sum_t most)
{
   spawn(*this, sub, most);

   return this;
}

node_t :: node_t (const node_t &parent)
{
   city_id_t x;

   matrix = parent.matrix;
   depth = parent.depth;
   optimal = parent.optimal;
   mic_list = new mic_list1_t[matrix->degree];
   for (x=0; x<matrix->degree; x++)
      mic_list[x] = parent.mic_list[x];
}
