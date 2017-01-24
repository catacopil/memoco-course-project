/* loss.cc
 *
 * orig ceh
 */
/*
#define PRINTIT
 */

#include "loss.h"
#include <assert.h>
#include "chnew.h"

int LossHeuristic :: can_run(const Matrix *m) const
{
   return m!=NULL;
}

LossHeuristic :: ~LossHeuristic ()
{
   delete sorted_cities;
   delete cities;
   delete from;
   if (to != from)
      delete to;
   to = NULL;
   from = NULL;
}

static int loss_cities_cmp(const void *p1, const void *p2)
{
   sum_t ret = (*(LossCity**)p2)->min_loss - (*(LossCity**)p1)->min_loss;

   return ( (ret < 0) ? -1 : ((ret == 0) ? 0 : 1) );
}

LossHeuristic :: LossHeuristic (const Matrix *m) : TourFinder(m)
{
   LossCity *lc;

   cost = 0;
   label = 0;
   to = new SortedMatrix(m, NO_ID);
   if (!m->is_symmetric()) {
      from = new SortedMatrix(m, MAX_DEGREE);
#ifdef PRINTIT
if (param.verbose) {
   from->print();
   to->print();
}
#endif
   }
   else
      from = to;
   endcities = (cities = new LossCity[degree]) + degree;
   sorted_cities = new LossCity*[degree];
   for (lc = cities; lc < endcities; lc++) {
      lc->label = lc->id = (city_id_t)(lc - cities);
      sorted_cities[lc->id] = lc;
      lc->to_sc = to->cost[lc->id];
      lc->from_sc = from->cost[lc->id];
      lc->from_id = lc->to_id = NO_ID;
   }
   for (lc = cities; lc < endcities; lc++)
      lc->find_loss(cities);
}

int LossHeuristic :: run()
{
   city_id_t i=2;
   LossCity *lc, *lc2;

   do {
      qsort(sorted_cities, degree, sizeof(LossCity*), loss_cities_cmp);
      lc = *sorted_cities;
      cost += lc->travel(cities, endcities);
   } while (++i < degree);
   for (lc2 = cities; lc2 < endcities; lc2++) {
      if (lc2->from_id == NO_ID) {
         for (lc = lc2; ; lc = cities+lc->to_id) {
            tour->travel(lc->id);
            if (lc->to_id == NO_ID)
               break;
         }
      }
   }
   return 0;
}

/* Point *1*: check for the finding of new best costs, first check each of
 * the two id (id and lid) being traveled for each of the four best costs
 * from and to each city, then check the the city that had it's label changed
 * (lid2) for each of the four cities, then check to see if each of the
 * cities changed or traveled are the actual city recognized.
 *
 * There might be an easier way to compare any 4 numbers to be equal to one of
 * three numbers, rather than doing 12 comparisons.
 */
cost_t LossCity :: travel(LossCity *cities, LossCity *endcities)
{
   LossCity *lc;
   city_id_t lid, lid2 = NO_ID;
   cost_t cost;

   lc = cities+(lid = choice_sc->id);
   cost = choice_sc->real_cost;
   if (status == LOSS_TO) {
      assert(lc->from_id == NO_ID);
      assert(to_id == NO_ID);
      lc->from_id = id;
      to_id = lid;
      if (from_id == NO_ID)
         label = lc->label;
      else {
         if (lc->to_id != NO_ID)
            while (lc->to_id != NO_ID)
               lc = cities+(lid2 = lc->to_id);
         lc->label = label;
      }
#ifdef PRINTIT
if (param.verbose)
   dump << "Traveling << " << id << "*-" << to_id << "\n";
#endif
   }
   else {
      assert(from_id == NO_ID);
      assert(lc->to_id == NO_ID);
      lc->to_id = id;
      from_id = lid;
      if (to_id == NO_ID)
         label = lc->label;
      else {
         if (lc->from_id != NO_ID)
            while (lc->from_id != NO_ID)
               lc = cities+(lid2 = lc->from_id);
         lc->label = label;
      }
#ifdef PRINTIT
if (param.verbose)
   dump << "Traveling << " << from_id << "-*" << id << "\n";
#endif
   }
   for (lc = cities; lc < endcities; lc++) {
      if (lc->to_choice_id == id || lc->from_choice_id == id
       || lc->to_sc->id == id || lc->from_sc->id == id
       || lc->to_choice_id == lid || lc->from_choice_id == lid
       || lc->to_sc->id == lid || lc->from_sc->id == lid
       || ((lid2 != NO_ID)
       ? (lc->to_choice_id == lid2 || lc->from_choice_id == lid2
       || lc->to_sc->id == lid2 || lc->from_sc->id == lid2) : 0)
       || lc->id == id || lc->id == lid || lc->id == lid2) /*1*/
         lc->find_loss(cities);
   }
   return cost;
}

/* Point *1*: here we impliment the flow chart in the Cruyssen paper.
 * before this point it is necessary to find the 3rd best costs.
 */
void LossCity :: find_loss(LossCity *cities)
{
   SortedCost *to_sc2, *from_sc2, *from_choice, *to_choice;
   LossCity *lc;
   sum_t from_loss, to_loss;

   if (to_id == NO_ID) {
      while (lc = cities+to_sc->id, lc->from_id != NO_ID
       || lc->label == label) {
         assert(to_sc->real_cost != MAX_COST);
         to_sc++;
      }
      for (to_sc2 = to_sc+1; lc = cities+to_sc2->id, (lc->from_id != NO_ID
        || lc->label == label) && to_sc2->real_cost != MAX_COST; to_sc2++)
         ;
      to_loss = to_sc2->real_cost - to_sc->real_cost;
      to_choice_id = to_sc2->id;
      to_choice = to_sc;
#ifdef PRINTIT
if (param.verbose)
   dump << id << " FoundLoss " << to_loss << " To " << to_sc->id << "\n";
#endif
   }
   else {
      to_loss = MIN_SUM;
      to_choice_id = NO_ID;
      to_choice = to_sc2 = NULL;
   }
   if (from_id == NO_ID) {
      while (lc = cities+from_sc->id, lc->to_id != NO_ID
       || lc->label == label) {
         assert(from_sc->real_cost != MAX_COST);
         from_sc++;
      }
      for (from_sc2 = from_sc+1; lc = cities+from_sc2->id, (lc->to_id != NO_ID
        || lc->label == label) && from_sc2->real_cost != MAX_COST; from_sc2++)
         ;
      from_loss = from_sc2->real_cost - from_sc->real_cost;
      from_choice_id = from_sc2->id;
      from_choice = from_sc;
#ifdef PRINTIT
if (param.verbose)
   dump << id << " FoundLoss " << from_loss << " From " << from_sc->id << "\n";
#endif
   }
   else {
      from_loss = MIN_SUM;
      from_choice_id = NO_ID;
      from_choice = from_sc2 = NULL;
   }
   if (from_id == to_id) {
      SortedCost *from_sc3, *to_sc3;
      assert(from_id == NO_ID);
      assert(from_sc2 != NULL && to_sc2 != NULL);
      to_sc3 = to_sc2 + ((to_sc2->real_cost != MAX_COST) ? 1 : 0);
      while (lc = cities+to_sc3->id, (lc->from_id != NO_ID
       || lc->label == label) && to_sc3->real_cost != MAX_COST)
         to_sc3++;
      from_sc3 = from_sc2 + ((from_sc2->real_cost != MAX_COST) ? 1 : 0);
      while (lc = cities+from_sc3->id, (lc->to_id != NO_ID
       || lc->label == label) && from_sc3->real_cost != MAX_COST)
         from_sc3++;
      assert(from_id == NO_ID);
      if (to_sc->id == from_sc->id) { /*1*/
         if (from_sc->real_cost + to_sc->real_cost
          > from_sc2->real_cost + to_sc2->real_cost) {
            from_loss = from_sc3->real_cost - from_sc2->real_cost;
            from_choice = from_sc2;
            if (to_sc2->id == from_sc2->id)
               to_loss = to_sc3->real_cost - to_sc->real_cost;
            else
               to_loss = to_sc2->real_cost - to_sc->real_cost;
         }
         else {
            to_loss = to_sc3->real_cost - to_sc2->real_cost;
            to_choice = to_sc2;
            if (from_sc2->id == to_sc2->id)
               from_loss = from_sc3->real_cost - from_sc->real_cost;
            else
               from_loss = from_sc2->real_cost - from_sc->real_cost;
         }
      }
      else {
         if (from_sc->id == to_sc->id)
            from_loss = from_sc3->real_cost - from_sc->real_cost;
         else
            from_loss = from_sc2->real_cost - from_sc->real_cost;
         if (to_sc->id == from_sc->id)
            to_loss = to_sc3->real_cost - to_sc->real_cost;
         else
            to_loss = to_sc2->real_cost - to_sc->real_cost;
      }
   }
   if (to_loss > from_loss) {
      min_loss = to_loss;
      status = LOSS_TO;
      choice_sc = to_choice;
   }
   else {
      min_loss = from_loss;
      status = LOSS_FROM;
      choice_sc = from_choice;
   }
   if (min_loss == MAX_SUM)
      min_loss = MIN_SUM;
}
