/* circlist.cc
 *
 * Base class for CircListElements and the CircList class which represents
 * a list of objects arranged in a circle.
 *
 * ceh
 */

#include "circlist.h"
#include "stdmacro.h"
#include <assert.h>

void CircList :: change_head_to(const CircListElement *here)
{
   assert(here->in_list == 1);
   head = (CircListElement*)here;
   tail = here->prev;
}

void CircList :: reverse()
{
   CircListElement *next = head, *save;

   if (next != NULL && next != tail) {
      for (save = NULL; save != head; next = save) {
         save = next->next;
         next->next = next->prev;
         next->prev = save;
      }
      save = head;
      head = tail;
      tail = save;
   }
}

CircListElement *CircList :: del(CircListElement *here)
{
   assert(listsize > 0);
   listsize--;
   if (here != NULL) {
      assert(here->in_list == 1 && head != NULL && here != NULL);
      here->in_list = 0;
      here->next->prev = here->prev;
      here->prev->next = here->next;
      if (here == head) {
         head = head->next;
         if (here == tail)
            head = tail = NULL;
      }
      if (here == tail)
         tail = tail->prev;
      here->next = here->prev = NULL;
   }
   return here;
}

long CircList :: size() const
{
   return listsize;
}

void CircList :: insert_sort(CircListElement *here,
   int cmp(const CircListElement*, const CircListElement*))
{
   CircListElement *start = head, *tmp;
   if ((tmp = start) == NULL) {
      insert(here);
      return;
   }
   for (tmp = tmp->prev; tmp != start; tmp = tmp->prev) {
      if (cmp(here, tmp) > 0) 
         break;
   }
   if (tmp != start) {
      insert_after(tmp, here);
   }
   else {
      if (cmp(here, tmp) > 0) 
         insert_after(tmp, here);
      else {
         insert(here);
         change_head_to(here);
      }
   }
}

void CircList :: insert(CircListElement *here)
{
   listsize++;
   assert(here->in_list == 0);
   here->in_list = 1;
   if (tail == NULL) {
      head = here;
   }
   else {
      tail->next = here;
      here->prev = tail;
   }
   head->prev = here;
   tail = here;
   here->next = head;
}

void CircList :: insert_after(CircListElement *after, CircListElement *here)
{
   listsize++;
   assert(here->in_list == 0);
   here->in_list = 1;
   if (tail == NULL) {
      head = tail = here->next = here->prev = here;
      return;
   }
   if (after == tail)
      tail = here;
   if (after == NULL) {
      after = tail;
      head = here;
   }
   after->next->prev = here;
   here->next = after->next;
   after->next = here;
   here->prev = after;
}

void CircListIter :: init(const CircList &cl)
{
   circlist = &cl;
   over_with = ((iter = cl.get_head()) == NULL);
}

CircListIter :: CircListIter(const CircList &cl)
{
   init(cl);
}

CircListElement *CircListIter :: next()
{
   CircListElement *here = NULL;

   if (!over_with) {
      if ((iter = (here = iter)->get_next()) == circlist->get_head())
         over_with = 1;
   }
   return here;
}

CircListIter :: ~CircListIter()
{
}

