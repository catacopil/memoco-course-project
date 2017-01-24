/* loss.h
 *
 * The Loss method is from the paper by P. Van der Cruyssend and M.J.Rijckaert
 * from Journal of the Operational Reseach Society Vol.29 No.7 p697-701
 *
 * orig implimenation ceh 11-30-94
 */
#ifndef _LOSS_H
#define _LOSS_H

#include "tourfind.h"
#include "smatrix.h"

#define LOSS_TO    0x0
#define LOSS_FROM  0x1

class LossCity
{
public:
   SortedCost *to_sc, *from_sc, *choice_sc;
   sum_t min_loss;
   city_id_t id, from_id, to_id, from_choice_id, to_choice_id, label;
   char status;
   void find_loss(LossCity *cities);
   cost_t travel(LossCity *, LossCity *);
};

class LossHeuristic : public TourFinder
{
   SortedMatrix *to, *from;
   LossCity *cities, *endcities, **sorted_cities;
   city_id_t label;
   sum_t cost;
public:
   LossHeuristic(const Matrix*);
   ~LossHeuristic();
   int can_run(const Matrix *) const;
   int run();
};

#endif
