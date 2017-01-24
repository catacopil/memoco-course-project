/* pqueue.h
 * a priority queue for branch and bound queues
 * orig ceh
 */

#ifndef _PQUEUE_H
#define _PQUEUE_H

#include "stdmacro.h"
#include "pquelem.h"
#include "binfile.h"

class TSPSolver;

class PQueue {
   PQuelem *head;
   PQuelem *lastdeq;
public:
   sum_t most;
   PQueue() { head = lastdeq = NULL; most = MAX_SUM; };
   ~PQueue();
   void dont_delete_last_dequeued() { lastdeq = NULL; };

   // standard enqueue and dequeue
   // enq returns if the element was enqueed or not
   int enq(PQuelem *);
   PQuelem *deq();

   // returns the Last element on the queue in the opposite direction the
   // priority is sorting by.  This is to take the largest element off first.
   // It is left up to the client do delete the element if it is used up
   // or re enque it if it is not.
   PQuelem *antideq();

   // use the sum_t value "most" as the order that can most be
   // returned or enqueed into this pqueue, this essentially means
   // no quelem should be accepted that has an order value higher than "most"
   // also calls clip on each element stripped, before it is freed
   void strip(sum_t most, void (*clip)(PQuelem*));

   // returns if the queue is empty
   int empty();

   // prints the queue into the Terminal
   void print(Term &t = dump) const;

   // read and write stuffs
   void write(BinFile &) const;
   void read(BinFile &, const PQuelem&, TSPSolver *);
};

#endif
