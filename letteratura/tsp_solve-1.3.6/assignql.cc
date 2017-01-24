/* assignql.cc
 *
 * Based on the paper by Carpaneto, Toth, ACM Transactions on Mathematical
 * Software, Vol. 6, No. 1, 1980, p104-111.
 *
 * All Points *X.x* commented as *X.x* represents steps in the paper's
 * algorithm.  Examples: 3.a would be Step 3a, 6.0 would begin Step 6.
 *
 * orig ceh 1992
 *  rewrite to model the 1980 paper ceh 12-7-94
 */

/*
#define PRINTIT_1
#define PRINTIT
#define PRINTH
#define SPONGE // Slows it down? why?
 */

#include "assignql.h"

#ifdef ITERS

inline void Zero_Set :: init_iter(zero_pointer &i)
{
   i = &start;
}

inline int Zero_Set :: init_iter(zero_pointer &i, city_id_t &j)
{
   if (start == 0)
      return 0;
   i = &start;
   j = COST_TO_CITY(start);
   return 1;
}

inline int Zero_Set :: next_iter(zero_pointer &i, city_id_t &j)
{
   assert (start);
   if (j != NO_ID && *(i = cost+j)) {
      j = COST_TO_CITY(*i);
   }
   else {
      i = &start;
      j = COST_TO_CITY(start);
   }
   return !cost[j];
}

inline int Zero_Set :: next_iter(zero_pointer &i)
{
   return *(i = cost+COST_TO_CITY(*i)) != 0;
}

inline int Zero_Set :: end_iter(zero_pointer &i)
{
   return *i == 0;
}

inline int Zero_Set :: empty()
{
   return start == 0;
}

inline int Zero_Set :: two_or_more()
{
   return (start == 0 || cost[COST_TO_CITY(start)] == 0) ? 0 : 1;
}

inline int Zero_Set :: ok(zero_pointer &i)
{
   int k = 0;
   zero_pointer j;
   for (init_iter(j); !end_iter(j); next_iter(j))
      if (j == i)
         k = 1;
   if (j == i)
      k = 1;
   return k;
}

inline void Zero_Set :: operator = (const Zero_Set &aps)
{
   // cost must be before assigned here
   assert(cost != NULL);
   start = aps.start;
}

Zero_Set :: Zero_Set()
{
   start = 0;
#ifndef NDEBUG
   cost = NULL;
#endif
}

inline zero_pointer Zero_Set :: assert_add(city_id_t i)
{
#ifdef PRINTIT_1
   dump << (void*)this << " zaA " << i << "\n";
#endif
#ifndef NDEBUG
   zero_pointer p = find(i);
   assert(end_iter(p));
   assert(cost[i] == 0);
#endif
   cost[i] = start;
   start = CITY_TO_COST(i);
   return &start;
}

inline void Zero_Set :: remove(zero_pointer p)
{
#ifdef PRINTIT_1
   dump << (void*)this << " zR " << *p << "\n";
#endif
   assert(ok(p));
   zero_pointer j;
   *p = *(j=(cost+COST_TO_CITY(*p)));
   *j = 0;
}

inline void Zero_Set :: remove(const city_id_t i)
{
   zero_pointer p = find(i);
   remove(p);
}

inline zero_pointer Zero_Set :: find(const city_id_t i)
{
   zero_pointer p;

   init_iter(p);
   if (!end_iter(p)) {
      do {
         if (COST_TO_CITY(*p) == i)
            break;
      } while (next_iter(p));
   }
   return p;
}

inline void Zero_Set :: if_exists_remove(const city_id_t i)
{
   zero_pointer p;
   if (!end_iter(p = find(i)))
      remove(p);
}

inline void Zero_Set :: remove_all_except(const city_id_t i, const city_id_t j)
{
#ifdef PRINTIT_1
   dump << (void*)this << " RAE " << i << "\n";
#endif
   if (i == NO_ID || j == NO_ID) {
      start = 0;
   }
   else {
      start = CITY_TO_COST(i);
      cost[i] = 0;
   }
}

inline void Zero_Set :: write(BinFile &t) const
{
   t << start;
}

inline void Zero_Set :: read(BinFile &t, cost_t *c)
{
   t >> start;
   init(c);
}

#endif

void AP_Set :: init_iter(czero_pointer &i)
{
   i = zs;
}

int AP_Set :: init_iter(czero_pointer &i, city_id_t &j)
{
   if (zsp == zs)
      return 0;
   i = zs;
   j = 0;
   return 1;
}

int AP_Set :: next_iter(czero_pointer &i, city_id_t &j)
{
   assert (zsp > zs);
   if (j == NO_ID) {
      j = 0;
      i = zs;
   }
   else {
      i = zs + ++j;
      if (i == zsp) {
         j = 0;
         i = zs;
      }
   }
   assert (i < zsp && i >= zs);
   if (i == zsp-1) {
      j = NO_ID;
      return 1;
   }
   return 0;
}

int AP_Set :: next_iter(czero_pointer &i)
{
   return ++i < zsp;
}

int AP_Set :: end_iter(czero_pointer &i)
{
   return i >= zsp;
}

int AP_Set :: empty()
{
   return zs == zsp;
}

int AP_Set :: two_or_more()
{
   return zsp > zs+1;
}

int AP_Set :: ok(czero_pointer &i)
{
   return i < zsp && i >= zs;
}

void AP_Set :: operator = (const AP_Set &aps)
{
#ifdef __BORLANDC__
   memcpy(zs, aps.zs, MAX_ZEROES*sizeof(city_id_t));
#else
   zs = aps.zs;
#endif
   zsp = zs + (city_id_t)(aps.zsp-aps.zs);
}

AP_Set :: AP_Set()
{
   zsp = zs;
}

inline czero_pointer AP_Set :: assert_add(city_id_t i)
{
#ifdef PRINTIT_1
   dump << (void*)this << " aA " << i << "\n";
#endif
#ifndef NDEBUG
   city_id_t *p = find(i);
   assert(p == zsp);
#endif
   assert(zsp < zs + MAX_ZEROES);
   *zsp++ = i;
   return zsp-1;
}

inline void AP_Set :: add_it(city_id_t i)
{
#ifdef PRINTIT_1
   dump << (void*)this << " A " << i << "\n";
#endif
   if (find(i) == zsp) {
      assert(zsp < zs + MAX_ZEROES);
      *zsp++ = i;
   }
}

inline void AP_Set :: remove(czero_pointer p)
{
#ifdef PRINTIT_1
   dump << (void*)this << " aR " << *p << "\n";
#endif
   assert(ok(p));
   *p = *--zsp;
}

inline void AP_Set :: remove(const city_id_t i)
{
   czero_pointer p = find(i);
   remove(p);
}

inline czero_pointer AP_Set :: find(const city_id_t i)
{
   city_id_t *p;
   for (p = zs; p < zsp; p++)
      if (*p == i)
         break;
   return p;
}

inline void AP_Set :: if_exists_remove(const city_id_t i)
{
   city_id_t *p = find(i);
   if (p < zsp)
      remove(p);
}

inline void AP_Set :: remove_all_except(const city_id_t i, const city_id_t j)
{
#ifdef PRINTIT_1
   dump << (void*)this << " RAE " << i << "\n";
#endif
   if (i == NO_ID || j == NO_ID) {
      zsp = zs;
   }
   else {
      zs[0] = i;
      zsp = zs+1;
   }
}

void AP_Set :: write(BinFile &t) const
{
   city_id_t sze = (city_id_t)(zsp-zs), *i;
   t << sze;
   for (i = (city_id_t*)zs; i < zsp; i++)
      t << *i;
}

void AP_Set :: read(BinFile &t, cost_t *)
{
   city_id_t sze, *i;
   t >> sze;
   zsp = zs+sze;
   for (i = zs; i < zsp; i++)
      t >> *i;
}

void anode_t :: initial_solution()
{
   city_id_t i;
   zero_pointer zsp1, zsp2;
   Zero_Set *zi, *zcj;

   i=0;
   for (; i<degree; i++) { /*3.e*/
      zi = &cities[i].zeroes; /*3.a*/
      zi->init_iter(zsp1);
      if (!zi->end_iter(zsp1)) {
         do {
            if (cities[COST_TO_CITY(*zsp1)].assmnt == NO_ID)
               break;
         } while (zi->next_iter(zsp1));
      }
      if (zi->end_iter(zsp1)) {
         zi->init_iter(zsp1);
         for (;;) {
            if (zi->end_iter(zsp1)) {
               solver->add_unassignment(i);
               goto end_loop;
            }
            zcj = &cities[cities[COST_TO_CITY(*zsp1)].assmnt].zeroes;
            zcj->init_iter(zsp2);
            if (!zcj->end_iter(zsp2)) { /*3.b*/
               do {
                  if (cities[COST_TO_CITY(*zsp2)].assmnt == NO_ID)
                     break;
               } while (zcj->next_iter(zsp2));
            }
            if (!zcj->end_iter(zsp2))
               break;
            zi->next_iter(zsp1);
         }
         cities[COST_TO_CITY(*zsp2)].assmnt
          = cities[COST_TO_CITY(*zsp1)].assmnt; /*3.c*/
         cities[cities[COST_TO_CITY(*zsp1)].assmnt].assnor
          = COST_TO_CITY(*zsp2);
/*
#ifdef ITERS
         zcj->remove(zsp2);
         zcj->assert_add(COST_TO_CITY(*zsp1));
#else
         *zsp2 = *zsp1;
#endif
 */
         zcj->remove(zsp2);
         zcj->assert_add(COST_TO_CITY(*zsp1));
      }
      cities[COST_TO_CITY(*zsp1)].assmnt = i;
      cities[i].assnor = COST_TO_CITY(*zsp1);
      zi->remove(zsp1);
      end_loop:;
   }
}

/* Point *1*: we need to check if the RH exists before we add so we call add_it.
 * (the second instance it actually should not exist, so it's good to
 * assert if debugging, but if it does (through more testing) exist we
 * might just use the add function.)
 *
 * Point *2*: by some tests i believe there shouldn't exist a zero here, so we
 * assert that there is not an already existing zero on the row.
 */
int anode_t :: find_min(AP_Set *RH, const sum_t most)
{
   cost_t min1 = MAX_COST, *cost;
   city_id_t *r, *c, *re = solver->rows+degree, *ce = solver->cols+degree;
   Zero_Set *z;

   for (r = solver->rows; r < re; r++) {
      if (*r == NO_ID)
         continue;
      cost = aprime->cost[(city_id_t)(r-solver->rows)];
      for (c = solver->cols; c < ce; c++) {
         if (*c != NO_ID)
            continue;
         if (min1 > cost[(city_id_t)(c-solver->cols)])
            min1 = cost[(city_id_t)(c-solver->cols)];
      }
   }
   assert(min1 < MAX_COST && min1 >= 0);
#ifdef PRINTIT_1
dump << min1 << "\n";
#endif
   if (min1 > 0) {
      if ((optimal += min1) >= most)
         return 1;
      for (r = solver->rows; r < re; r++) {
         cost = aprime->cost[(city_id_t)(r-solver->rows)];
         z = &cities[(city_id_t)(r-solver->rows)].zeroes;
         if (*r != NO_ID) {
            for (c = solver->cols; c < ce; c++) {
               if (*c != NO_ID)
                  continue;
               if ((cost[(city_id_t)(c-solver->cols)] -= min1) == 0) {
                  z->assert_add((city_id_t)(c-solver->cols)); /*2*/
                  RH->add_it((city_id_t)(r-solver->rows)); /*1*/
               }
            }
         }
         else {
            for (c = solver->cols; c < ce; c++) {
               if (*c == NO_ID)
                  continue;
#ifdef ITERS
#else
#endif
               z->if_exists_remove((city_id_t)(c-solver->cols));
               cost[(city_id_t)(c-solver->cols)] += min1;
            }
         }
      }
   }
   else {
      for (r = solver->rows; r < re; r++) {
         cost = aprime->cost[(city_id_t)(r-solver->rows)];
         if (*r != NO_ID) {
            for (c = solver->cols; c < ce; c++) {
               if (*c != NO_ID)
                  continue;
#ifdef ITERS
               if (cost[(city_id_t)(c-solver->cols)] == 0) /*FIX: may work*/
#else
               if (cost[(city_id_t)(c-solver->cols)] == 0)
#endif
                  RH->assert_add((city_id_t)(r-solver->rows)); /*1*/
            }
         }
      }
   }
#ifdef PRINTIT_1
aprime->print();
#endif
   return 0;
}

/* assume that r is unlabeled to be assigned
 */
void anode_t :: assignment(city_id_t r, const sum_t most)
{
   city_id_t *_i, *_iend;
   zero_pointer l;
   czero_pointer rp;
   AP_Set RH;

   for (_i=solver->rows, _iend = _i+degree*3; _i < _iend; )
      *_i++ = NO_ID;
   solver->rows[r] = MAX_DEGREE;
   while (cities[r].zeroes.init_iter(l, solver->next[r])) { /*4.a*/
      if (cities[r].zeroes.two_or_more())
         RH.assert_add(r);
      label_four_b: /*4.b*/
      assert(cities[r].zeroes.ok(l));
      if (solver->cols[COST_TO_CITY(*l)] != NO_ID) {
         if (rp = RH.find(r), !RH.end_iter(rp))
            goto label_four_d;
         break; /* goto label_four_c; */
      }
      solver->cols[COST_TO_CITY(*l)] = r;
      if (cities[COST_TO_CITY(*l)].assmnt == NO_ID)
         goto label_six;
      r = cities[COST_TO_CITY(*l)].assmnt;
      solver->rows[r] = COST_TO_CITY(*l);
   }
   if (!RH.empty()) { /*4.c*/
      RH.init_iter(rp);
      r = *rp;
      label_four_d:
      if (cities[r].zeroes.next_iter(l, solver->next[r])) { /*4.d*/
         RH.remove(rp);
      }
      goto label_four_b;
   }
   if (find_min(&RH, most)) { /*5.0*/
#ifdef NDEBUG
      solver->unassigned = NO_ID;
#else
      while (solver->unassigned != NO_ID)
         solver->remove_unassignment(solver->unassigned);
#endif
      return;
   }
   RH.init_iter(rp);
   r = *rp;
   goto label_four_d;
   label_six:
   cities[COST_TO_CITY(*l)].assmnt = r;
   cities[r].assnor = COST_TO_CITY(*l);
   cities[r].zeroes.remove(l);
   while (solver->rows[r] != MAX_DEGREE) {
      assert(solver->rows[r] != NO_ID);
      l = cities[r].zeroes.assert_add(solver->rows[r]);
      assert(COST_TO_CITY(*l) == solver->rows[r]);
      r = solver->cols[COST_TO_CITY(*l)];
      cities[COST_TO_CITY(*l)].assmnt = r;
      cities[r].assnor = COST_TO_CITY(*l);
      cities[r].zeroes.remove(COST_TO_CITY(*l));
   }
   solver->remove_unassignment(r);
}

void apcity_t :: write(BinFile &t) const
{
   t << assmnt << assnor << next << prev;
   zeroes.write(t);
}

void apcity_t :: read(BinFile &t, cost_t *c)
{
   t >> assmnt >> assnor >> next >> prev;
   zeroes.read(t, c);
}

void anode_t :: write(BinFile &t) const
{
   apcity_t *ml = cities;

   t << depth << optimal << included << unincluded;
   aprime->write(t);
   for (ml = cities; ml < mic_end; ml++)
      ml->write(t);
}

PQuelem *anode_t :: read_clone(BinFile &t, TSPSolver *s) const
{
   apcity_t *ml = cities;
   anode_t *o = new anode_t(s->matrix, s);

   t >> o->depth >> o->optimal >> o->included >> o->unincluded;
   o->aprime->read(t);
   for (ml = o->cities; ml < o->mic_end; ml++)
      ml->read(t, aprime->cost[(city_id_t)(ml-o->cities)]);
   return o;
}

void anode_t :: base_construct(const Matrix *m, TSPSolver *s)
{
   apcity_t *ml;

#ifdef NO_ARRAY_CONSTRUCTING
   assert(0);
#endif
   degree = m->degree;
   assert(m->degree == degree);
   solver = (APSolver*)s;
   depth = 1;
   matrix = m;
   mic_end = (ml = cities = new apcity_t[degree]) + degree;
   aprime = new Matrix(*matrix);

   for (; ml < mic_end; ml++) {
      ml->assmnt = ml->assnor = NO_ID;
      ml->next = (city_id_t)(ml-cities)+1;
      ml->prev = (city_id_t)(ml-cities)-1;
#ifdef ITERS
      ml->zeroes.init(aprime->cost[(city_id_t)(ml-cities)]);
#endif
   }
   cities->prev = MAX_DEGREE;
   mic_end[-1].next = NO_ID;
   included = NO_ID;
   unincluded = 0;
   optimal = 0;
}

void anode_t :: exclude(city_id_t id, city_id_t to)
{
   apcity_t *ml = cities+id;
   cost_t *c;

#ifdef PRINTIT
dump << "Excluding " << id << "->" << to << "\n";
#endif
   assert(!cities[to].is_included());
#ifdef ITERS
   if (*(c = aprime->cost[id]+to) <= 0 && ml->assnor != to)
#else
   if (*(c = aprime->cost[id]+to) == 0 && ml->assnor != to)
#endif
      cities[id].zeroes.remove(to);
   if (ml->assnor == to)
      solver->add_unassignment(id);
   *c = MAX_COST;
   if (ml->assnor != NO_ID) {
      cities[ml->assnor].assmnt = NO_ID;
      ml->assnor = NO_ID;
   }
}

void anode_t :: include(city_id_t id, city_id_t to)
{
   city_id_t i;
   apcity_t *ml = cities+id, *mlto = cities+to;
   cost_t *c;

   if (mlto->is_included()) {
      assert(mlto->assmnt == id);
#ifdef PRINTIT
dump << "Skiping Include " << id << "->" << to << "\n";
#endif
      return;
   }
#ifdef PRINTIT
dump << "Including " << id << "->" << to << "\n";
#endif
#ifdef ITERS
   cities[id].zeroes.remove_all_except((aprime->cost[id][to] <= 0
#else
   cities[id].zeroes.remove_all_except((aprime->cost[id][to] == 0
#endif
      && ml->assnor != to) ? to : NO_ID, ml->assnor);
   for (i = 0; i < degree; i++) {
      if (i != id) {
         c = aprime->cost[i]+to;
#ifdef ITERS
         if (i != mlto->assmnt && *c <= 0)
#else
         if (i != mlto->assmnt && *c == 0)
#endif
            cities[i].zeroes.remove(to);
         *c = MAX_COST;
      }
      if (i != to)
         aprime->cost[id][i] = MAX_COST;
   }
   if (ml->assnor != to) {
      cities[id].zeroes.assert_add(ml->assnor);
      solver->add_unassignment(mlto->assmnt);
      if (mlto->assmnt != NO_ID) {
         cities[mlto->assmnt].assnor = NO_ID;
         mlto->assmnt = NO_ID;
      }
      solver->add_unassignment(id);
      if (ml->assnor != NO_ID) {
         cities[ml->assnor].assmnt = NO_ID;
         ml->assnor = NO_ID;
      }
   }
   assert(!mlto->is_included());
   if (mlto->next != NO_ID)
      cities[mlto->next].prev = mlto->prev;
   if (mlto->prev != MAX_DEGREE) {
      assert(cities[mlto->prev].prev != NO_ID);
      cities[mlto->prev].next = mlto->next;
   }
   else
      unincluded = mlto->next;
   mlto->next = included;
   included = to;
   mlto->prev = NO_ID;
   depth++;
}

void anode_t :: impede(const sum_t, const city_id_t)
{
}

void anode_t :: make_assignment(const city_id_t id, const city_id_t to)
{
   city_id_t *ass;
   if (*(ass = &cities[to].assnor) != NO_ID)
      cities[*ass].assmnt = NO_ID;
   *ass = id;
   if (*(ass = &cities[id].assmnt) != NO_ID)
      cities[*ass].assnor = NO_ID;
   *ass = to;
}

anode_t :: anode_t (const anode_t &parent, const city_id_t sub,
   const city_id_t k, Tour *t, const sum_t most)
{
   city_id_t count, last;
   Path *p;

   set_equal(parent);
   p = t->get_head();
   for (count = 0; count < sub; count++, p = p->get_next())
      ;
   for (count = degree; last = p->id, --count > k; ) {
      p = p->get_next();
      include(last, p->id);
      make_assignment(last, p->id);
   }
   while (solver->unassigned != NO_ID)
      assignment(solver->unassigned, most);
   order(optimal, depth);
}

anode_t :: anode_t (const Matrix *m, TSPSolver *s)
{
   base_construct(m, s);
}

anode_t :: anode_t (const Matrix *m, TSPSolver *s, const sum_t most)
{
   city_id_t i, j;
   cost_t min1, *cost_toi, *end;

   base_construct(m, s);
   for (i=0; i<degree; i++) {
      cost_toi = aprime->cost[i];
      min1 = *cost_toi;
      end = cost_toi+degree;
      while (++cost_toi < end) {
         if (min1 > *cost_toi)
            min1 = *cost_toi;
      }
      cost_toi -= degree+1;
      while (++cost_toi < end)
         *cost_toi -= min1;
      optimal += min1;
   }
   for (i = 0; i < degree; i++) {
      min1 = aprime->val(0, i);
      for (j = 1; j < degree; j++) {
         if (min1 > aprime->val(j, i))
            min1 = aprime->val(j, i);
      }
      for (j = 0; j < degree; j++) {
         if ((aprime->cost[j][i] -= min1) == 0) {
#ifdef PRINTIT_1
dump << j << " Zero at " << i << "\n";
#endif
            cities[j].zeroes.assert_add(i);
         }
      }
      optimal += min1;
   }
#ifdef PRINTIT
aprime->print();
#endif
   initial_solution();
   while (solver->unassigned != NO_ID) /*4.0*/
      assignment(solver->unassigned, most);
   if (matrix->is_oneway()) {
      include(degree-1, degree-2);
      while (solver->unassigned != NO_ID)
         assignment(solver->unassigned, most);
   }
   if (param.verbose) {
      dump << "ap=" << optimal << "\n";
      for (optimal = 0, i = 0; i<degree; i++)
         optimal += matrix->val(cities[i].assmnt, i);
      dump << "ap=" << optimal << "\n";
   }
   order(optimal, depth);
}

anode_t *anode_t :: rebirth(const city_id_t from,
   const city_id_t to, const sum_t most)
{
   include(from, to);
   while (solver->unassigned != NO_ID)
      assignment(solver->unassigned, most);
   order(optimal, depth);
   return this;
}

anode_t :: anode_t (const anode_t &parent, const sum_t most,
   const city_id_t from, const city_id_t to)
{
   city_id_t prev, next;

   set_equal(parent);
   prev = from, next = cities[prev].assnor;
   for (; next != to; prev = next, next = cities[prev].assnor) {
      include(prev, next);
   }
   exclude(prev, next);
   while (solver->unassigned != NO_ID)
      assignment(solver->unassigned, most);
   order(optimal, depth);
}

anode_t :: anode_t (const anode_t &parent, const city_id_t from,
   const city_id_t to, const sum_t most)
{
   set_equal(parent);
   include(from, to);
   while (solver->unassigned != NO_ID)
      assignment(solver->unassigned, most);
   order(optimal, depth);
}

anode_t :: anode_t()
{
   aprime = NULL;
   cities = NULL;
}

anode_t :: ~anode_t()
{  
   if (aprime != NULL) {
      delete aprime;
      aprime = NULL;
   }
   if (cities != NULL) {
      delete cities;
      cities = NULL;
   }
}

void anode_t :: set_equal (const anode_t &parent)
{
   apcity_t *ml, *pml;

   solver = parent.solver;
   degree = parent.degree;
   matrix = parent.matrix;
   depth = parent.depth;
   optimal = parent.optimal;
   mic_end = (ml = cities = new apcity_t[degree]) + degree;
   included = parent.included;
   unincluded = parent.unincluded;
   aprime = new Matrix(*parent.aprime);
   for (pml = parent.cities; ml < mic_end; ml++, pml++) {
      ml->assmnt = pml->assmnt;
      ml->assnor = pml->assnor;
      ml->next = pml->next;
      ml->prev = pml->prev;
#ifdef ITERS
      ml->zeroes.init(aprime->cost[(city_id_t)(ml-cities)]);
#endif
      ml->zeroes = pml->zeroes;
   }
}

anode_t :: anode_t (const anode_t &parent)
{
   set_equal(parent);
   order(optimal, depth);
}

void anode_t :: printn() const
{
   apcity_t *city;
   sum_t add = 0;

   for (city = cities; city < mic_end; city++) {
      dump << "  (" << (city_id_t)(city-cities) << ") asi="
         << city->assmnt << " asnor=" << city->assnor;
      if (city->assmnt != NO_ID) {
         dump << "  (" << (city_id_t)(city-cities) << ")<-{" << city->assmnt
            << "},[" << matrix->val(city->assmnt, (city_id_t)(city-cities))
            << "]";
         add += matrix->val(city->assmnt, (city_id_t)(city-cities));
      }
      dump << "\n";
   }
#ifdef PRINTIT
aprime->print();
#endif
   dump << "cost=" << add << "," << optimal << "\n";
}

void anode_t :: branch(PQueue &que)
{
   apcity_t *ml;
   city_id_t init = (param.initial_choice == NO_ID) ? 0 : param.initial_choice;
   city_id_t inc = (included != NO_ID) ? included : init;
#ifdef PRINTIT
anode_t *apn;

   dump << "       TOP " << inc << "\n";
   printn();
#endif

   for (ml = cities; ml < mic_end; ml++) {
      if (ml->prev == NO_ID || inc == (city_id_t)(ml-cities)
       || init == (city_id_t)(ml-cities) )
         continue;
#ifdef PRINTIT
      dump << "   Slush   " << inc << ", " << (city_id_t)(ml-cities) << "\n";
      if (que.enq(apn = new anode_t (*this, inc, (city_id_t)(ml-cities),
       que.most)))
         apn->printn();
#else
      que.enq( new anode_t (*this, inc, (city_id_t)(ml-cities), que.most) );
#endif
      /*
      que.enq( rebirth(inc, (city_id_t)(ml-cities), que.most) );
      que.dont_delete_last_dequeued();
      ml = mic_end;
       */
   }
}

void anode_t :: fast_branch(PQueue &que)
{
   city_id_t next, i, shortest_next = unincluded;
   apcity_t *ml;
#ifdef PRINTIT
anode_t *apn;
#endif

#ifdef SPONGE
   next = cities[unincluded].assnor;
   for (i=1; next != unincluded; next = cities[next].assnor) {
      i++;
   }
   if (i == degree) {
      depth = degree-2;
      que.enq(new anode_t (*this));
      return;
   }
   ml = cities + unincluded;
   assert(unincluded != ml->assnor);
#ifdef PRINTIT
   dump << "       TOP " << unincluded << "\n";
   printn();
#endif
#else
   city_id_t shortest = MAX_DEGREE, next2, cycles = 0;
#ifdef NDEBUG
   city_id_t *_i, *_iend;
   for (_i = solver->Unassigned, _iend = _i+degree; _i < _iend; )
      *_i++ = NO_ID;
#endif
   for (next = unincluded; next != NO_ID; next = cities[next].next) {
      if (solver->Unassigned[next] == NO_ID) {
         solver->Unassigned[next] = 0;
         ml = cities+next;
         next2 = ml->assnor;
         assert(!ml->is_included());
         for (i=1; next2 != next; ) {
            solver->Unassigned[next2] = 0;
            ml = cities + next2;
            next2 = ml->assnor;
            if (!ml->is_included())
               i++;
         }
         if (i < shortest) {
            shortest = i;
            shortest_next = next;
         }
         cycles++;
      }
   }
#ifndef NDEBUG
   city_id_t *_i, *_iend;
   for (_i = solver->Unassigned, _iend = _i+degree; _i < _iend; )
      *_i++ = NO_ID;
#endif
   assert(shortest_next != NO_ID);
   assert(cycles > 0);
// dump << "Cycs " << cycles << "\n";
   if (cycles == 1) {
      depth = degree-2;
      que.enq(new anode_t (*this));
      return;
   }
#ifdef PRINTIT
   dump << "       TOP " << shortest_next << "\n";
   printn();
#endif
   ml = cities + shortest_next;
   assert(shortest_next != ml->assnor);
#endif
   do {
      next = ml->assnor;
      ml = cities+next;
      if (ml->is_included())
         continue;
#ifdef PRINTIT
      dump << "   Slush   " << shortest_next << ", " << next << "\n";
      if (que.enq(apn = new anode_t (*this, que.most, shortest_next, next)))
         apn->printn();
#else
      que.enq(new anode_t (*this, que.most, shortest_next, next));
#endif
   } while (next != shortest_next);
}

static int compare_TL(const CircListElement *p1, const CircListElement *p2)
{
   TourListElement *t1=(TourListElement*)p1, *t2=(TourListElement*)p2;
   return t1->tour.size() - t2->tour.size();
}

class alabel_t {
public:
   city_id_t id, assmnt;
   char label;
   alabel_t() {};
   void init(const apcity_t *m, const city_id_t i) {
      id = i;
      assmnt = m->assnor;
      label = 1;
   }
};

// patching looks at the cycles of the AP and pair by pair patches them
// together minimizing the patch cost to end up with a tour
void anode_t :: construct_tour(Tour *tour, int sort) const
{
   CircList tourlist;
   TourListElement *tl, *t2l;
   Path *p, *p2, *save, *save2;
   apcity_t *ml;
   sum_t best, tmp, tmp2;
   cost_t *outcost;
   alabel_t *alabels = new alabel_t[degree], *enda, *a, *a2;
   city_id_t cycles=0;

   save = save2 = NULL;
   for (enda = (a = alabels) + degree, ml = cities; a < enda; ml++, a++)
      a->init(ml, (city_id_t)(ml-cities));
   for (a = alabels; a < enda; a++) {
      if (a->label) {
         cycles++;
         a2 = alabels+a->assmnt;
         tl = new TourListElement;
         if (sort)
            tourlist.insert_sort(tl, compare_TL);
         else
            tourlist.insert(tl);
         tl->tour.travel(a->id);
         a->label = 0;
#ifdef PRINTH
if (param.verbose)
dump << " " << a->id;
#endif
         for (; a2->id != a->id; a2 = alabels+a2->assmnt) {
            tl->tour.travel(a2->id);
            a2->label = 0;;
#ifdef PRINTH
if (param.verbose)
dump << " " << a2->id;
#endif
         }
#ifdef PRINTH
if (param.verbose)
dump << "\n";
#endif
      }
   }
   delete alabels;
   CircListIter ci;
   TourIter c2, c3;
   ci.init(tourlist);
   tl = (TourListElement*)ci.next();
   assert(tl != NULL);
   while ((t2l = (TourListElement*)ci.next()) != NULL) {
      best = MAX_SUM;
      for (c2.init(t2l->tour); (p2 = c2.next()) != NULL; ) {
         outcost = matrix->cost[p2->get_prev()->id];
         tmp = -outcost[p2->id];
         for (c3.init(tl->tour); (p = c3.next()) != NULL; ) {
            if (best > (tmp2 = tmp + matrix->val(p->get_prev()->id, p2->id)
             + outcost[p->id] - matrix->val(p->get_prev()->id, p->id))) {
               best = tmp2;
               save = p;
               save2 = p2;
            }
         }
      }
      assert(best != MAX_SUM);
#ifdef PRINTH
if (param.verbose>1)
dump << "Found Best " << best << " " << save->id << " " << save2->id << "\n";
#endif
      tl->tour.change_head_to(save);
      t2l->tour.change_head_to(save2);
      tl->tour.append(&t2l->tour);
   }
   tour->copy(tl->tour);
   ci.init(tourlist);
   while ((t2l = (TourListElement*)ci.next()) != NULL)
      delete t2l;
}
