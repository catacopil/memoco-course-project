/* pqueue.cc
 *
 * priority queue base class element
 * orig ceh
 */

#include "io.h"
#include "pqueue.h"
#include "params.h"

PQueue :: ~PQueue()
{
   PQuelem *t;

   while ((t = head) != NULL) {
      head = t->next;
      delete t;
   }
   if (lastdeq != NULL) {
      delete lastdeq;
      lastdeq = NULL;
   }
}

void PQueue :: print(Term &term) const
{
   PQuelem *t;

   if (lastdeq != NULL) {
      term << "Handling: L" << lastdeq->ord.id << ","
         << lastdeq->ord.sum << "\n";
      /*
      lastdeq->print();
       */
   }
   for (t = head; t != NULL; t = t->next) {
      term << "L" << t->ord.id << ","
         << t->ord.sum << "\n";
      /*
      t->print();
       */
   }
}

void PQueue :: write(BinFile &term) const
{
   PQuelem *t;
   long elems;
   for (elems = 0, t = head; t != NULL; t = t->next)
      elems++;
   term << elems << most;
   for (t = head; t != NULL; t = t->next) {
      t->write(term);
      t->ord.write(term);
   }
}

void PQueue :: read(BinFile &term, const PQuelem &dummy, TSPSolver *solver)
{
   PQuelem *t = NULL;
   long elems;
   this->PQueue::~PQueue();
   for (head = NULL, term >> elems >> most; elems > 0; elems--) {
      if (head == NULL)
         t = head = dummy.read_clone(term, solver);
      else
         t = t->next = dummy.read_clone(term, solver);
      t->ord.read(term);
      t->next = NULL;
   }
}

int PQueue :: enq(PQuelem *data)
{
   PQuelem **t;

   if (data->ord.sum >= most) {
      delete data;
      return 0;
   }
   for (t = &head; *t != NULL && **t < *data; t = &(*t)->next)
      ;
   data->next = *t;
   *t = data;
   return 1;
}

PQuelem *PQueue :: antideq()
{
   PQuelem **t, *e;

   if (lastdeq != NULL) {
      delete lastdeq;
      lastdeq = NULL;
   }
   for (t = &head; *t != NULL && (*t)->next != NULL; t = &(*t)->next)
      ;
   e = *t;
   *t = NULL;
   return e;
   /*
   assert((t == &head && head != NULL) ? head->next == NULL : 1);
   return (t == &head) ? NULL : e;
    */
}

PQuelem *PQueue :: deq()
{
   if (lastdeq != NULL)
      delete lastdeq;
   if ((lastdeq = head) != NULL)
      head = head->next;
   return lastdeq;
}

// use the sum_t value "most" as the order that can most be
// returned or enqueed into this PQueue, this essentially means
// no PQuelem should be accepted that has an order value higher than "most"
void PQueue :: strip(sum_t the_most, void (*clip)(PQuelem*))
{
   PQuelem **t, **n, *j;

   most = the_most;
   for (t = &head; *t != NULL; t = n) {
      n = &(*t)->next;
      if ((*t)->ord.sum >= most) {
         j = *t;
         *t = *n;
         if (clip != NULL)
            clip(j);
         delete j;
         n = t;
      }
   }
}

int PQueue :: empty()
{
   return head == NULL;
}
