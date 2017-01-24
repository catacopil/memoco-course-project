/* oneacity.cc
 *
 * cities in a onetree
 *
 * orig ceh
 */
#include "oneacity.h"

#ifdef MAX_FORBIDDEN

void forbidden_cities :: forbid(const city_id_t j)
{
   city_id_t *i;
   for (i = forbidden; *i != NO_ID; i++)
      ;
   *i++ = j;
   assert(i-forbidden < MAX_FORBIDDEN);
   *i = NO_ID;
}

forbidden_cities_array :: ~forbidden_cities_array()
{
   forbidden_cities *l;
   if (list == NULL)
      return;
   for (l = list; l < end_city; l++)
      l->forbidden_cities::~forbidden_cities();
   delete (char*)list;
}

int forbidden_cities :: not_forbidden(const city_id_t j) const
{
   city_id_t *i;
   for (i = forbidden; *i != NO_ID; i++)
      if (*i == j)
         return 0;
   return 1;
}

forbidden_cities :: ~forbidden_cities()
{
   forbidden[0] = NO_ID;
}

// C E h
void forbidden_cities :: read(BinFile &t)
{
   city_id_t *i, fsize;
   t >> fsize;
   for (i = forbidden; fsize-- > 0; i++) {
      assert((i-forbidden) < MAX_FORBIDDEN);
      t >> *i;
   }
   assert((i-forbidden) < MAX_FORBIDDEN);
   *i = NO_ID;
}

void forbidden_cities :: write(BinFile &t) const
{
   city_id_t *i, fsize = 0;
   for (i = (city_id_t*)forbidden; *i != NO_ID; i++)
      fsize++;
   t << fsize;
   for (i = (city_id_t*)forbidden; *i != NO_ID; i++)
      t << *i;
}

void forbidden_cities :: operator = (const forbidden_cities &c)
{
   const city_id_t *i;
   for (i = c.forbidden; *i != NO_ID; i++)
      forbid(*i);
}

city_id_t forbidden_cities :: iter(void *&j)
{
   if (j == NULL)
      j = forbidden;
   else
      ((city_id_t*)j) += 1;
   return (*((city_id_t*)j) == NO_ID) ? (j = NULL, NO_ID) : *((city_id_t*)j);
}

#else

city_id_t forbidden_cities :: iter(void *&j)
{
   if (j == NULL) {
      j = forbidden;
   }
   else
      j = ((forbidden_city*)j)->next;
   return (j == NULL) ? NO_ID : ((forbidden_city*)j)->id;
}

void forbidden_cities :: forbid(const city_id_t j)
{
   forbidden_city *i = new forbidden_city;

//dump << this << " A" << j << "\n";
   i->id = j;
   i->next = forbidden;
   forbidden = i;
}

int forbidden_cities :: not_forbidden(const city_id_t j) const
{
   forbidden_city *i;

   for (i = forbidden; i != NULL; i = i->next)
      if (i->id == j)
         return 0;
   return 1;
}

forbidden_cities :: ~forbidden_cities()
{
   forbidden_city *i, *p;
   for (i = forbidden; i != NULL; i = p) {
      p = i->next;
      delete i;
   }
   forbidden = NULL;
}

// C E h
void forbidden_cities :: read(BinFile &t)
{
   city_id_t fsize, i;

   this->forbidden_cities::~forbidden_cities();
   t >> fsize;
   while (fsize-- > 0) {
      t >> i;
      forbid(i);
   }
}

void forbidden_cities :: write(BinFile &t) const
{
   forbidden_city *i;
   city_id_t fsize = 0;
   
   for (i = forbidden; i != NULL; i = i->next)
      fsize++;
   t << fsize;
   for (i = forbidden; i != NULL; i = i->next)
      t << i->id;
}

void forbidden_cities :: operator = (const forbidden_cities &c)
{
   const forbidden_city *i;
   this->forbidden_cities::~forbidden_cities();
   for (i = c.forbidden; i != NULL; i = i->next)
      forbid(i->id);
}

#endif

void city_a :: read(BinFile &t)
{
   fs.read(t);
   t >> section >> branches >> lambda;
#ifdef SECORD
   t >> secord_val;
#endif
   switch (section) {
   case 1: t >> u.sec1.required_edge1 >> u.sec1.required_edge2; break;
   case 4: case 2: t >> u.sec2.required_edge >> u.sec2.onetree_edge
    >> u.sec2.end; break;
   case 3: t >> u.sec3.onetree_edge; break;
   default: assert(0);
   }
}

void city_a :: write(BinFile &t) const
{
   fs.write(t);
   t << section << branches << lambda;
#ifdef SECORD
   t << secord_val;
#endif
   switch (section) {
   case 1: t << u.sec1.required_edge1 << u.sec1.required_edge2; break;
   case 4: case 2: t << u.sec2.required_edge << u.sec2.onetree_edge
    << u.sec2.end; break;
   case 3: t << u.sec3.onetree_edge; break;
   default: assert(0);
   }
}

void city_a :: operator = (const city_a &c)
{
   this->city_a::~city_a();
   u = c.u;
   branches = c.branches;
   section = c.section;
   lambda = c.lambda;
#ifdef SECORD
   secord_val = c.secord_val;
#endif
   fs = c.fs;
}

void city_a :: place(AMatching *matchings, AMatching *& lclean, city_a *ml)
{
   switch (section) {
   case 1:
      matchings[get_id(ml)].place_branch(matchings, u.sec1.required_edge1,
         lclean);
      matchings[get_id(ml)].place_branch(matchings, u.sec1.required_edge2,
         lclean);
      break;
   case 4:
   case 2:
      matchings[get_id(ml)].place_branch(matchings, u.sec2.required_edge,
         lclean);
      if (u.sec2.onetree_edge != NO_ID) {
         matchings[get_id(ml)].place_branch(matchings, u.sec2.onetree_edge,
            lclean);
         matchings[u.sec2.onetree_edge].place_branch(matchings, get_id(ml),
            lclean);
      }
      break;
   case 3:
      if (u.sec3.onetree_edge != NO_ID) {
         matchings[get_id(ml)].place_branch(matchings, u.sec3.onetree_edge,
            lclean);
         matchings[u.sec3.onetree_edge].place_branch(matchings, get_id(ml),
            lclean);
      }
      break;
   default: assert(0);
   }
}

void AMatching :: find_clean_branch(AMatching *list, city_id_t b, city_id_t p,
   AMatching *& last_clean)
{
   AMatching *l;

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
void AMatching :: place_branch(AMatching *list, city_id_t b,
   AMatching *& last_clean)
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
      AMatching *tmp = this;
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

void AMatching :: print(AMatching *list)
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
      AMatching *tmp = this;
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

void AMatching :: travel(AMatching *list, city_id_t b)
{
   assert(branches > 0 && b != NO_ID);
   if (b == b1 && !(traveled & B1TRAVELED))
      traveled |= B1TRAVELED;
   else if (b == match && !(traveled & MATRAVELED))
      traveled |= MATRAVELED;
   else if (branches > 1 && b == b2 && !(traveled & B2TRAVELED))
      traveled |= B2TRAVELED;
   else {
      AMatching *tmp = this;
      assert(branches > 2 && tmp->next != MAX_DEGREE);
      do {
         tmp = list + tmp->next;
         assert(tmp->branches == 1 && (tmp->next == MAX_DEGREE
            ? !(tmp->b2 != b || (tmp->traveled & B2TRAVELED)) : 1));
      } while (tmp->b2 != b || (tmp->traveled & B2TRAVELED));
      tmp->traveled |= B2TRAVELED;
   }
}

AMatching *AMatching :: find_next_untraveled(AMatching *list)
{
   AMatching *tmp;

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

void AMatching :: depth_first(AMatching *list, Tour *tour, Path *par, int pr)
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
      AMatching *tmp = this;
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
