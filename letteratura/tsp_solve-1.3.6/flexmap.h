/* flexmap.h
 *
 * Finds a tour according to the paper by Bernd Fritzke and Peter Wilke
 * using the Flexmap algorithm described within, the paper is located in
 * IJCNN-91 Singapore.  The code is written by Chad Hurwitz.
 *
 * orig ceh
 */
#ifndef _FLEXMAP_H
#define _FLEXMAP_H

#include "tourfind.h"
#include "circlist.h"
#include "matrix.h"

class FMCell : public CircListElement {
   short initialized;
public:
   pos_t pos;
   double error;
   city_id_t pinned_city;

   FMCell();
   void move_towards(const pos_t &, const double epsilon);
   inline double operator - (const pos_t &);
   inline FMCell *get_next() { return (FMCell*)CircListElement::get_next(); }
   inline FMCell *get_prev() { return (FMCell*)CircListElement::get_prev(); }
};

class FMCity {
public:
   short pins;
   inline void operator = (const FMCity &fmc)
      { pins = fmc.pins, pos.x = fmc.pos.x, pos.y = fmc.pos.y, id = fmc.id; };
   FMCell *bmu;
   FMCity();
   pos_t pos;
   city_id_t id;
   int find_new_bmu(const short half_neighbors);
   void move_bmu(CircList &, const float, const float);
};

class FlexMapHeuristic : public TourFinder {
   float Ebmu, Eneighbor;
   short Ndistribution, Kneighbor, Npins;
   double *error;
   FMCity *cities;
   CircList cells;
   city_id_t num_pinned_cities;
public:
   FlexMapHeuristic (const Matrix*);
   ~FlexMapHeuristic();
   int can_run(const Matrix *) const;
   int run(void);
};

#endif
