/* circlist.h
 *
 * Base class for CircListElements and the CircList class which represents
 * a list of objects arranged in a circle.
 *
 * ceh
 */
#ifndef _CIRCLIST_H
#define _CIRCLIST_H

#include "stdmacro.h"

class CircListElement {
friend class CircList;
   _uchar in_list;
protected:
   CircListElement *next;
   CircListElement *prev;
public:
   CircListElement() { in_list = 0; };
   inline CircListElement *get_next() const { return next; };
   inline CircListElement *get_prev() const { return prev; };
};

class CircList {
   CircListElement *head, *tail;
protected:
   long listsize;
public:
   inline ~CircList() {
      head = tail = NULL;
      listsize = 0;
   };
   long size() const;
   inline CircList() { this->CircList::~CircList(); };
   inline int empty() const { return tail == NULL; };

   // deletes the element and returns it, if the argument is NULL nothing haps
   CircListElement *del(CircListElement *);

   // insert in ascending order depending on the sign of the return value of cmp
   void insert_sort(CircListElement *,
      int cmp(const CircListElement*, const CircListElement*));
   void insert(CircListElement *);
   void insert_after(CircListElement*after_this, CircListElement*insert_this);

   inline CircListElement *get_head() const { return head; };
   inline CircListElement *get_tail() const { return tail; };

   // changes the head to the circlist element pointer passed
   // so that iterators will start at this Elem instead of the current head
   void change_head_to(const CircListElement *);

   void reverse();
};

class CircListIter {
   const CircList *circlist;
   CircListElement *iter;
   short over_with;
public:
   // iterates starting at the head and ending at the tail
   // after the tail is returned from the iterator, NULL is returned
   // on the following call.
   void init(const CircList &);
   CircListIter() {};
   CircListIter(const CircList &);
   CircListElement *next();
   ~CircListIter();
};

#endif
