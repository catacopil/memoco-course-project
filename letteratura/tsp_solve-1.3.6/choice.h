/* choice.h
 *
 * fun loving user interface for how to choose tour finders 
 *
 * orig ceh
 */

#ifndef _CHOICE_H
#define _CHOICE_H

#include "tourfind.h"
#include "matrix.h"
#include "tour.h"

typedef struct {
//   const _uchar *abrev;
   const char *abrev;
   const char *name;
   /*
   long times;
   double cost;
    */
   short temp;
} choice;

extern choice choices[];

extern TourFinder *switch_tourfinders(short x, const Matrix *m);

extern TourFinder *switch_tourimprovers(short x, const Matrix *m, Tour *t);

extern TourFinder *switch_parralleltourfinders(short x, const Matrix *m);

#endif _CHOICE_H
