/* multfrag.h
 *
 * A watered down adaptation of the multiple fragment algorithm from
 * Experiments on Geometric TSP heuristics Computing Science Technical
 * Report No. 151 AT&T Bell Labs 1990.  The basic concept is you sort
 * the costs of all edges, and piece together a tour while iterating
 * edges in order.
 *
 * There are seemingly endless tangents that can be built from this
 * idea.  You basically transform the matrix and/or modify it as each
 * edge is chosen and you have a new algorithm.  One method is to
 * transformed into costs = [3*Cij - CIj - CiJ - Cji], where i,j
 * are the end cities of the edge and I is the closest city to j
 * and J is the closest city from i.  This is BestInOutHeuristic.
 *
 * orig implimentation ceh
 */
#ifndef _MULTFRAG_H
#define _MULTFRAG_H

#include "tourfind.h"
#include "pqueue.h"
#include "smatrix.h"

// used to sort the edges
class SCEntry : public PQuelem
{
public:
   SortedCost *sc;
   city_id_t id;
   ~SCEntry();
   inline SCEntry(SortedCost **c, city_id_t i)
   {
      sc = c[i];
      id = i;
      order(sc->real_cost, 0);
   };
   inline void enq(PQueue &q)
   {
      order((++sc)->real_cost, 0);
      q.enq(this);
      q.dont_delete_last_dequeued();
   };
};

class MFLabel
{
public:
   city_id_t degree, label, to;
};

class OrderedCost
{
public:
   city_id_t to;
   cost_t cost;
   inline OrderedCost() { to = NO_ID; cost = MAX_COST; };
};

class MultiFragHeuristic : public TourFinder
{
   MFLabel *labels;
   city_id_t count;
   PQueue cost_que;
   virtual void cleanup(const city_id_t i, const city_id_t j);
protected:
   SortedMatrix *sm;
   inline MultiFragHeuristic(const Matrix *m, int) : TourFinder (m) {};
   void construct();
public:
   MultiFragHeuristic(const Matrix*);
   ~MultiFragHeuristic ();
   int can_run(const Matrix *) const;
   int run();
};

class TransformHeuristic : public MultiFragHeuristic
{
public:
   TransformHeuristic(const Matrix *m);
};

class BestInOutHeuristic : public MultiFragHeuristic
{
   virtual void cleanup(const city_id_t i, const city_id_t j);
   OrderedCost *bestin, *bestout;
public:
   BestInOutHeuristic(const Matrix *m);
   ~BestInOutHeuristic();
};

#endif
