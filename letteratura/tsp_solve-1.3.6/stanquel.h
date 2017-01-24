/* stanquel.h
 *
 * the standard least arcs que element
 *
 * orig ceh
 */

#ifndef _STANQUEL_H
#define _STANQUEL_H

#include "stdmacro.h"
#include "pquelem.h"
#include "matrix.h"

#define NOT_IMPEDED ((char)1)
#define IMPEDED ((char)0)
#define LAST_NOT_IMPEDED ((char)2)

class mic_list1_t {
public:
   inline mic_list1_t() {};
   inline void operator = (const mic_list1_t&m)
      { mic_id = m.mic_id, orig = m.orig, cost = m.cost, impeded = m.impeded; };
   city_id_t mic_id;
   city_id_t orig;
   cost_t cost;
   _uchar impeded;
};

class node_t : public PQuelem {

   void spawn(const node_t &, const city_id_t, const sum_t);

public:
   ~node_t() { delete mic_list; };

   // The initial node maker to start the branching tour
   node_t (city_id_t, const Matrix *); 

   // The node maker to invoke a new node from a currently constructed branch
   node_t (const node_t &, const city_id_t, const sum_t most);

   // Similer to the new inocation of a child but, destroy's the current
   // object and assigns its "last" child over it, returning the addres to this
   // as the new "reborn" child node, who encompasses the same space the parent
   // did.
   node_t *rebirth(const city_id_t, const sum_t most);

   // simple copy constructor
   node_t (const node_t &);

   // return quickly whether or not the sub problem x of this node_t will
   // be possible to travel.  If impeded returns non-zero then the subproblem
   // doesn't need to be looked at further.
   inline char impeded(const city_id_t sub, const sum_t most)
   {
      return (optimal - mic_list[sub].cost + matrix->val(mic_list[depth].mic_id,
         mic_list[sub].mic_id) >= most) ? IMPEDED : NOT_IMPEDED;
   };

   // simple clone (allocate and copy and return pointer to clone)
   PQuelem * clone() const { return new node_t(*this); };

   // element printing for quelem print
   void print() const;

   const Matrix *matrix;
   sum_t optimal;                /* optimum completion value */
   city_id_t depth;              /* for the depth in the queue */

   mic_list1_t *mic_list;        /* THE mic_lists of the node */
};

#endif
