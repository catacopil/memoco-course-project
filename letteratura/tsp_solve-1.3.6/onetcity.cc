/* onetcity.cc
 *
 * cities in a onetree
 *
 * orig ceh
 */

#include "onetcity.h"

/* Point *1*: only swap if the pointer is not this
 */
void city_t :: degrade_section(city_t *(& cptr),
   const city_id_t new_required_edge)
{
   city_t swap(*cptr);

   assert(section == 2 && cptr->section == 2);
   *cptr = *this;
   cptr->u.sec1.required_edge1 = u.sec2.required_edge;
   cptr->u.sec1.required_edge2 = new_required_edge;
   cptr->section--;
   if (cptr++ != this) /*1*/
      *this = swap;
}

void city_t :: degrade_section(city_t *(& cptr),
   const city_id_t new_required_edge, const city_id_t end)
{
   city_t swap(*cptr);

   assert(section == 3 && cptr->section == 3);
   *cptr = *this;
   cptr->u.sec2.onetree_edge = u.sec3.onetree_edge;
   cptr->u.sec2.required_edge = new_required_edge;
   cptr->u.sec2.end = end;
   cptr->section--;
   if (cptr++ != this) /*1*/
      *this = swap;
}

city_id_t city_t :: tree_id() const
{
   if (section == 2) {
      if (u.sec2.end > id)
         return u.sec2.end;
      return id;
   }
   return id;
}

// same as up except for if it's section2 and NO_ID it returns the end.up()
// and it returns a unique labeling tree_id of the node or partial tour.
// c E  h
city_id_t city_t :: travel_up(const city_t **micmap) const
{
   const city_t *city;
   if (section == 2) {
      if (u.sec2.onetree_edge == NO_ID)
         city = micmap[u.sec2.end];
      else
         city = this;
      if (city->u.sec2.onetree_edge == NO_ID)
         return NO_ID;
      city = micmap[city->u.sec2.onetree_edge];
   }
   else if (u.sec3.onetree_edge != NO_ID)
      city = micmap[u.sec3.onetree_edge];
   else
      return NO_ID;
   return city->tree_id();
}

void city_t :: read(BinFile &t)
{
   t >> section >> id >> branches;
   switch (section) {
   case 1: t >> u.sec1.required_edge1 >> u.sec1.required_edge2; break;
   case 2: t >> u.sec2.required_edge >> u.sec2.onetree_edge>>u.sec2.end; break;
   case 3: t >> u.sec3.onetree_edge; break;
   default: assert(0); break;
   }
}

void city_t :: write(BinFile &t) const
{
   t << section << id << branches;
   switch (section) {
   case 1: t << u.sec1.required_edge1 << u.sec1.required_edge2; break;
   case 2: t << u.sec2.required_edge << u.sec2.onetree_edge<<u.sec2.end; break;
   case 3: t << u.sec3.onetree_edge; break;
   default: assert(0); break;
   }
}

lambda_cost_t city_t :: find_cost(city_id_t i, const SortedMatrix *sm) const
{
   city_id_t i2;
   for (i2=0; i2<sm->degree; i2++) {
      if (i == sm->cost[id][i2].id)
         break;
   }
   assert(i2 != sm->degree);
   return sm->cost[id][i2].cost;
}

void city_t :: place(Matching *matchings, Matching *& lclean)
{
   switch (section) {
   case 1:
      matchings[id].place_branch(matchings, u.sec1.required_edge1, lclean);
      matchings[id].place_branch(matchings, u.sec1.required_edge2, lclean);
      break;
   case 2:
      matchings[id].place_branch(matchings, u.sec2.required_edge, lclean);
      if (u.sec2.onetree_edge != NO_ID) {
         matchings[id].place_branch(matchings, u.sec2.onetree_edge, lclean);
         matchings[u.sec2.onetree_edge].place_branch(matchings, id, lclean);
      }
      break;
   case 3:
      if (u.sec3.onetree_edge != NO_ID) {
         matchings[id].place_branch(matchings, u.sec3.onetree_edge, lclean);
         matchings[u.sec3.onetree_edge].place_branch(matchings, id, lclean);
      }
      break;
   default: assert(0); break;
   }
}

void Matching :: find_clean_branch(Matching *list, city_id_t b, city_id_t p,
   Matching *& last_clean)
{
   Matching *l;

   for (l = last_clean; l->next != NO_ID || l->branches >= 2; l++)
      ;
   last_clean = l+1;
   if ((l->next = next) != MAX_DEGREE)
      list[next].u.previous = (city_id_t)(l-list);
   list[p].next = (city_id_t)(l-list);
   l->b2 = b;
   l->u.previous = p;
}

// Point *1*: insert along chain for branches>2
// Point *2*: remove chain if going from 1 to 2 and chain exists
void Matching :: place_branch(Matching *list, city_id_t b,
   Matching *& last_clean)
{
   if (branches < 2) {
      if (branches++) {
         if (next != NO_ID) {
            find_clean_branch(list, b2, u.previous, last_clean); /*2*/
            next = NO_ID;
         }
         b2 = b;
      }
      else
         b1 = b;
   }
   else { /*1*/
      Matching *tmp = this;
      if (next == NO_ID) {
         assert(branches == 2);
         next = MAX_DEGREE;
      }
      else {
         assert(next != MAX_DEGREE && branches > 2);
         do {
            tmp = list+tmp->next;
         } while (tmp->next != MAX_DEGREE);
      }
      tmp->find_clean_branch(list, b, (city_id_t)(tmp-list), last_clean);
      branches++;
   }
}

void Matching :: print(Matching *list)
{
   dump << (city_id_t)(this - list) << " has ";
   if (branches == 0) {
      dump << " NO Branches\n";
      return;
   }
   dump << branches << "'s  b1 ="
      << ((traveled & B1TRAVELED) ? "*" : "") << "= " << b1;
   if (match != NO_ID)
      dump << "  match =" << ((traveled & MATRAVELED) ? "*" : "") << "= "
       << match;
   if (branches > 1)
      dump << "  b2 =" << ((traveled & B2TRAVELED) ? "*" : "") << "= " << b2;
   if (branches > 2) {
      Matching *tmp = this;
      assert(tmp->next != MAX_DEGREE);
      do {
         tmp = list + tmp->next;
         dump << "  b~ =" << ((tmp->traveled & B2TRAVELED) ? "*" : "") << "= "
          << tmp->b2;
         assert(tmp->branches == 1);
      } while (tmp->next != MAX_DEGREE);
   }
   dump << "\n";
}

void Matching :: travel(Matching *list, city_id_t b)
{
   assert(branches > 0 && b != NO_ID);
   if (b == b1 && !(traveled & B1TRAVELED))
      traveled |= B1TRAVELED;
   else if (b == match && !(traveled & MATRAVELED))
      traveled |= MATRAVELED;
   else if (branches > 1 && b == b2 && !(traveled & B2TRAVELED))
      traveled |= B2TRAVELED;
   else {
      Matching *tmp = this;
      assert(branches > 2 && tmp->next != MAX_DEGREE);
      do {
         tmp = list + tmp->next;
         assert(tmp->branches == 1 && (tmp->next == MAX_DEGREE
            ? !(tmp->b2 != b || (tmp->traveled & B2TRAVELED)) : 1));
      } while (tmp->b2 != b || (tmp->traveled & B2TRAVELED));
      tmp->traveled |= B2TRAVELED;
   }
}

Matching *Matching :: find_next_untraveled(Matching *list)
{
   Matching *tmp;

   if (!(traveled & B1TRAVELED)) {
      traveled |= B1TRAVELED;
      tmp = list+b1;
   }
   else if (match != NO_ID && !(traveled & MATRAVELED)) {
      traveled |= MATRAVELED;
      tmp = list+match;
   }
   else if (branches > 1 && !(traveled & B2TRAVELED)) {
      traveled |= B2TRAVELED;
      tmp = list+b2;
   }
   else if (branches > 2) {
      tmp = this;
      assert(tmp->next != MAX_DEGREE);
      do {
         tmp = list+tmp->next;
         assert(tmp->branches == 1);
      } while ((tmp->traveled & B2TRAVELED) && tmp->next != MAX_DEGREE);
      if (tmp->traveled & B2TRAVELED)
         return NULL;
      tmp->traveled |= B2TRAVELED;
      tmp = list+tmp->b2;
   }
   else
      return NULL;
   tmp->travel(list, (city_id_t)(this-list));
   return tmp;
}

void Matching :: depth_first(Matching *list, Tour *tour, Path *par, int pr)
{
   if (traveled & CITRAVELED)
      return;
   traveled |= CITRAVELED;
   Path *child = tour->insert_after(par, (city_id_t)(this-list));

if (pr)
dump << "Traveling " << (city_id_t)(this-list) << "\n";
   if (par != NULL)
      travel(list, par->id);
   if (!(traveled & B1TRAVELED)) {
      traveled |= B1TRAVELED;
      list[b1].depth_first(list, tour, child, pr);
   }
   if (match != NO_ID && !(traveled & MATRAVELED)) {
      traveled |= MATRAVELED;
      list[match].depth_first(list, tour, child, pr);
   }
   if (branches > 1 && !(traveled & B2TRAVELED)) {
      traveled |= B2TRAVELED;
      list[b2].depth_first(list, tour, child, pr);
   }
   if (branches > 2) {
      Matching *tmp = this;
      assert(tmp->next != MAX_DEGREE);
      do {
         tmp = list+tmp->next;
         assert(tmp->branches == 1);
         if (!(tmp->traveled & B2TRAVELED)) {
            tmp->traveled |= B2TRAVELED;
            list[tmp->b2].depth_first(list, tour, child, pr);
         }
      } while ( tmp->next != MAX_DEGREE);
   }
}
