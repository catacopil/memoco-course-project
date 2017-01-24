/* tour.h
 * the tour class
 * orig ceh
 */

#ifndef _TOUR_H
#define _TOUR_H

#include "matrix.h"
#include "circlist.h"
#include "binfile.h"

class Path : public CircListElement {
public:
   city_id_t id;
   inline Path* get_next() { return (Path*)CircListElement::get_next(); }
   inline Path* get_prev() { return (Path*)CircListElement::get_prev(); }
};

class TourIter;

class Tour {
friend TourIter;
   city_id_t degree; // total possible cities
   Path *static_paths;
   CircList circ;
   Path *get_new_path(const city_id_t);
   sum_t length_cost;
   void construct(const city_id_t degree);
public:
   Tour();
   void copy(const Tour &);
   Tour(const city_id_t degree);
   ~Tour();

   // add a path traveled along the tour
   void travel(city_id_t);

   Path *insert_after(Path *, city_id_t);
   void insert_before(city_id_t before_here, city_id_t here);

   // returns non-zero if the tour is a complete Tour
   // has degree number of paths traveled and travels each id exactally
   // once
   int is_complete(const city_id_t deg) const;
   int is_complete() const;

   // returns if less than "degree" number of cities was traveled
   inline int not_full() const { return circ.size() != degree; }
   inline city_id_t size() const { return (city_id_t)circ.size(); }

   // prints the path
   void print() const;
   void print(Term &term) const;
   void print_oneway(Term &term) const;

   void write(BinFile &term) const;
   void read(BinFile &term);
   void write(Term &term) const;
   void read(Term &term);

   // show the positions
   void show(const Matrix *m) const;

   sum_t cost(const Matrix *m);

   // make new head, assert h is already in list
   void change_head_to(const city_id_t h);
   void change_head_to(const Path *);

   Path *get_head() const;

   // Appends the tour passed to this tour inserting the tour
   // between the head and tail of this tour, while removing them from
   // the passed tour.
   void append(Tour *);

   // reverses the tour (changes head to tail and all inbetween)
   void reverse();

   // returns 0 if this tour and T are equal, non-zero if different
   int compare(Tour &T);
};

class TourIter : public CircListIter {
public:
   inline TourIter() {};
   inline TourIter(const Tour &t) : CircListIter(t.circ) {}
   inline void init(const Tour &t) { CircListIter::init(t.circ); }
   inline Path* next() { return (Path*)CircListIter::next(); }
};

class TourListElement : public CircListElement {
public:
   Tour tour;
};

#endif
