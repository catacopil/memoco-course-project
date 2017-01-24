/* smatrix.cc
 *
 * sorted matrix for use with the 1-tree solver and anyone else that cares to
 * listen.
 *
 * orig ceh
 */

#include "smatrix.h"
#include <assert.h>
#include <math.h>

#define MATRIXSTACK_UNIQ 0x4A741857L
/*
#define PRINTM
 */

void SortedMatrix :: operator = (const SortedMatrix &sm)
{
   city_id_t x;
   assert(degree == sm.degree);
   for (x = 0; x < degree; x++) {
      memcpy(cost[x], sm.cost[x], sizeof(SortedCost)*degree);
      lambdas[x] = sm.lambdas[x];
   }
   matrix = sm.matrix;
   status = sm.status;
   smid = 0;
   Exclusion *ex;
   while (exclusions != NULL) {
      ex = exclusions->next;
      delete exclusions;
      exclusions = ex;
   }
   exclusions = NULL;
}

void SortedMatrix :: write(BinFile &t) const
{
   city_id_t x, y, last, now;
   long exes;
   SortedCost *sc;
   Exclusion *ex;

   t << degree << smid << status;
   for(x = 0; x < degree; x++) {
      sc = cost[x];
      for (y = 0; y < degree; y++)
         t << sc[y].cost << sc[y].real_cost << sc[y].id;
   }
   for(x = 0; x < degree; x++)
      t << lambdas[x];
   for (exes = 0, ex = exclusions; ex != NULL; ex = ex->next)
      exes++;
   t << exes;
   for (ex = exclusions; ex != NULL; ex = ex->next) {
      sc = cost[ex->x];
      last = (city_id_t)(ex->last_pos - sc);
      now = (city_id_t)(ex->now_pos - sc);
      t << ex->sorted_cost.cost << ex->sorted_cost.real_cost
         << ex->sorted_cost.id << last << now << ex->x << ex->smid;
   }
}

inline void SortedMatrix :: construct(city_id_t deg)
{
   city_id_t x;
   degree = deg;
   cost = new SortedCost *[degree];
   lambdas = new lambda_t [degree];
   for (x = 0; x < degree; x++)
      cost[x] = new SortedCost[degree];
   status = 0;
   smid = 0;
   exclusions = NULL;
}

SortedMatrix :: SortedMatrix(const Matrix *m, city_id_t j)
{
   construct(m->degree);
   sort(m, j);
}

SortedMatrix :: SortedMatrix(BinFile &t, const Matrix *ma)
{
   city_id_t x, y;
   SortedCost *sc;
   long exes;
   Exclusion *ex=NULL;

   matrix = ma;
   t >> degree;
   construct(degree);
   t >> smid >> status;
   for (x = 0; x < degree; x++) {
      sc = cost[x];
      for (y = 0; y < degree; y++)
         t >> sc[y].cost >> sc[y].real_cost >> sc[y].id;
   }
   for (x = 0; x < degree; x++)
      t >> lambdas[x];
   exclusions = NULL;
   for (t >> exes; exes > 0; exes--) {
      if (exclusions == NULL) 
         ex = exclusions = new Exclusion;
      else
         ex = ex->next = new Exclusion;
      t >> ex->sorted_cost.cost >> ex->sorted_cost.real_cost
         >> ex->sorted_cost.id >> x >> y >> ex->x >> ex->smid;
      ex->last_pos = cost[ex->x]+x; /* FIX: don't know if it'll work */
      ex->now_pos = cost[ex->x]+y;
      ex->next = NULL;
   }
}

SortedMatrix :: SortedMatrix(city_id_t deg)
{
   construct(deg);
}

SortedMatrix :: SortedMatrix(const SortedMatrix &sm)
{
   construct(sm.degree);
   *this = sm;
}

SortedMatrix ::~SortedMatrix()
{
   city_id_t x;
   Exclusion *ex;

   while (exclusions != NULL) {
      ex = exclusions->next;
      delete exclusions;
      exclusions = ex;
   }
   x=degree;
   do {
      delete cost[--x];
   } while (x != 0);
   delete lambdas;
   delete cost;
}

static void print_cost(SortedCost *sc)
{
   if (sc->real_cost == MAX_COST)
      dump << " MAX-";
   else
      dump.form("%4d-", (int)sc->real_cost);
   if (fequiv(sc->cost, MAX_LCOST, MAX_LCOST/100))
      dump << "MAX ";
   else
      dump.form("%-4d", (int)sc->cost);
   dump << "(" << sc->id << ")";
}

void SortedMatrix :: print()
{
   city_id_t x, y, deg = (city_id_t)((degree-1 < 7) ? (degree-1) : 7);
   for (x = 0; x < degree; x++) {
      for (y = 0; y < deg; y++) {
         print_cost(cost[y]+x);
         dump << ",";
      }
      print_cost(cost[y]+x);
      dump << "\n";
   }
}

/* Warning: This must return 0 if the costs are equal.  Otherwise, with the
 * commented code below, the same two elements passed in opposite orders
 * can return 1 and -1, which will make qsort crash on some machines.
 */
static int scost_cmp(const void *p1, const void *p2)
{
   lambda_cost_t ret = ((SortedCost*)p1)->cost - ((SortedCost*)p2)->cost;
   return ( (ret < 0) ? -1 : ((ret == 0) ? 0 : 1) );
}

void SortedMatrix :: sort(const Matrix *m, city_id_t j)
{
   city_id_t x, y;
   SortedCost *c;
   cost_t *co, *jco;

   matrix = m;
   if (degree != m->degree)
      return;
   for (x = 0; x < degree; x++) {
      c = cost[x];
      lambdas[x] = 0;
      switch (j) {
      case MAX_DEGREE:
         for (y = 0; y < degree; y++) {
            c[y].cost = c[y].real_cost = m->val(y,x);
            c[y].id = y;
         }
         break;
      case NO_ID:
         co = m->cost[x];
         for (y = 0; y < degree; y++) {
            c[y].cost = c[y].real_cost = co[y];
            c[y].id = y;
         }
         break;
      default:
         jco = m->cost[j];
         co = m->cost[x];
         for (y = 0; y < degree; y++) {
            if (j == y)
               c[y].cost = MAX_LCOST;
            else
               c[y].cost = co[y] - co[j] - jco[y];
            c[y].real_cost = co[y];
            c[y].id = y;
         }
         break;
      }
      c[x].cost = MAX_LCOST;
      qsort(c, degree, sizeof(SortedCost), scost_cmp);
   }
   status = 1;
}

inline void SortedMatrix :: resort(const city_id_t x, const lambda_t *lams)
{
   SortedCost *data, *end, *sorted, *sorted1, swap, *c;

   data = c = cost[x];
   data->cost = data->real_cost + lams[x] + lams[data->id];
   /*
   for (end = c+degree-1; ++data < end && data->cost != MAX_COST; ) {
    */
   for (end = c+degree-1; ++data < end; ) {
      data->cost = data->real_cost + lams[x] + lams[data->id];
      for (sorted1=sorted=data; --sorted >= c; sorted1--) {
         if (sorted1->cost > sorted->cost)
              break;
         swap = *sorted;
         *sorted = *sorted1;
         *sorted1 = swap;
      }
   }
}

void SortedMatrix :: resort(const city_id_t i)
{
   resort(i, this->lambdas);
}

void SortedMatrix :: sort(const Matrix *m, const lambda_t *lams)
{
   city_id_t x;

   assert(status);
   if (degree != m->degree)
      return;
   for (x = 0; x < degree; x++)
      resort(x, lams);
}

smid_t SortedMatrix :: new_smid()
{
   return smid++;
}

void SortedMatrix :: forbid_edge(smid_t smid, city_id_t x, city_id_t y)
{
   city_id_t i;
   SortedCost *sc=cost[x];
   Exclusion *ex=new Exclusion;

   ex->x = x;
   ex->smid = smid;
   for (i = 0; i < degree; i++, sc++) {
      if (sc->id == y)
         break;
   }
   assert(i < degree);
   ex->sorted_cost = *sc;
   ex->last_pos = sc;
   for (sc++; sc->real_cost != MAX_COST; sc++)
      sc[-1] = *sc;
   ex->now_pos = --sc;
   sc->cost = MAX_LCOST;
   sc->real_cost = MAX_COST;
   sc->id = ex->sorted_cost.id;
   ex->next = exclusions;
   exclusions = ex;
}

void SortedMatrix :: restore(smid_t sm_id)
{
   SortedCost *sc;
   Exclusion *ex;

   while (exclusions->smid > sm_id) {
      for (sc = exclusions->now_pos; sc >= exclusions->last_pos; sc--)
         *sc = sc[-1];
      *sc = exclusions->sorted_cost;
      ex = exclusions->next;
      delete exclusions;
      exclusions = ex;
   }
}

#define MAXRECURSESTACKS 3
typedef struct sm_stack {
   SortedMatrix *sm;
   void *restored_sm;
   struct sm_stack *next;
   short users; // the number of nodes currently using this matrix
} sm_stack;
static sm_stack *static_ss, *st_stacks[MAXRECURSESTACKS];
static int recurse_stacks = 0;

// this procedure shouldn't assign static_ss == NULL because
// it could be called recursively
void init_matrix_stack(const city_id_t deg)
{
   install_blocked_mallocs(sizeof(SortedCost)*deg, deg/4+2);
   assert(recurse_stacks != MAXRECURSESTACKS);
   st_stacks[recurse_stacks++] = static_ss;
   static_ss = NULL;
}

void free_matrix_stack()
{
   sm_stack *ss;
   for (; static_ss != NULL; static_ss = ss) {
      ss = static_ss->next;
      dump << static_ss->sm << " still used by " << static_ss->users << "\n";
      delete static_ss->sm;
      delete static_ss;
   }
   assert(recurse_stacks > 0);
   static_ss = st_stacks[--recurse_stacks];
}

void write_matrix_stack(BinFile &t)
{
   long matricies;
   sm_stack *ss;
   
#ifndef OLD_TIMER
   matricies = MATRIXSTACK_UNIQ;
   t << matricies;
#endif
   for (matricies = 0, ss = static_ss; ss != NULL; ss = ss->next)
      matricies++;
   t << matricies;
   for (ss = static_ss; ss != NULL; ss = ss->next) {
      t << ss->sm << ss->users;
      ss->sm->write(t);
   }
#ifndef OLD_TIMER
   matricies = MATRIXSTACK_UNIQ;
   t << matricies;
#endif
}

void read_matrix_stack(BinFile &t, const Matrix *ma)
{
   assert(static_ss == NULL);
   long matricies;
   sm_stack *ss=NULL;

#ifndef OLD_TIMER
   t >> matricies;
   die_if(matricies != MATRIXSTACK_UNIQ, "", "");
#endif
   assert(static_ss == NULL);
   for (t >> matricies; matricies > 0; matricies--) {
      if (static_ss == NULL)
         ss = static_ss = new sm_stack;
      else
         ss = ss->next = new sm_stack;
      t >> ss->restored_sm >> ss->users;
      ss->sm = new SortedMatrix(t, ma);
   }
   if (ss != NULL)
      ss->next = NULL;
#ifndef OLD_TIMER
   t >> matricies;
   die_if(matricies != MATRIXSTACK_UNIQ, "", "");
#endif
}

SortedMatrix *find_SortedMatrix_ptr(const void *v)
{
   sm_stack *ss;
   for (ss = static_ss; ss != NULL; ss = ss->next) {
      if (ss->restored_sm == v)
         break;
   }
   return ss->sm;
}

SortedMatrix *start_using_matrix(const SortedMatrix &sm)
{
   sm_stack *ss = static_ss, copy;
   copy.sm = new SortedMatrix(sm);
   copy.next = ss;
   copy.users = 1;
   static_ss = new sm_stack;
   *static_ss = copy;

#ifdef PRINTM
dump << "Started1 " << static_ss->sm << " " << static_ss->sm->lambdas[0]
   << " " << static_ss->sm->lambdas[1] << " " << static_ss->sm->lambdas[2]
   << " " << static_ss->sm->lambdas[3] << " " << static_ss->sm->lambdas[4]
   << "\n";
#endif
   return static_ss->sm;
}

void use_matrix(SortedMatrix *sm)
{
   sm_stack **ss=&static_ss, *s;

   if ((s=static_ss)==NULL)
      assert(0);
   while ((*ss)->sm != sm) {
      ss = &(*ss)->next;
      assert(*ss != NULL);
   }
   if (*ss != static_ss) {
      static_ss = *ss;
      *ss = (*ss)->next;
      static_ss->next = s;
   }
   static_ss->users++;
#ifdef PRINTM
dump << "Used   " << static_ss->users << " " << static_ss->sm
   << " " << static_ss->sm->lambdas[0] << " " << static_ss->sm->lambdas[1]
   << " " << static_ss->sm->lambdas[2] << " " << static_ss->sm->lambdas[3]
   << " " << static_ss->sm->lambdas[4] << "\n";
#endif
}

void unuse_matrix(SortedMatrix *sm)
{
   sm_stack **ss=&static_ss, *s;
   if ((s=static_ss)==NULL)
      assert(0);
   while ((*ss)->sm != sm) {
      ss = &(*ss)->next;
      assert(*ss != NULL);
   }
   if (*ss != static_ss) {
      static_ss = *ss;
      *ss = (*ss)->next;
      static_ss->next = s;
   }
#ifdef PRINTM
dump << "Unused " << static_ss->users-1 << " " << static_ss->sm
   << " " << static_ss->sm->lambdas[0] << " " << static_ss->sm->lambdas[1]
   << " " << static_ss->sm->lambdas[2] << " " << static_ss->sm->lambdas[3]
   << " " << static_ss->sm->lambdas[4] << "\n";
#endif
   if (static_ss->users-- == 1) {
      s = static_ss->next;
      delete static_ss->sm;
      delete static_ss;
      static_ss = s;
   }
}

void make_lone_matrix(SortedMatrix **sm)
{
   sm_stack *ss;
   SortedMatrix *savesm=*sm;
   if ((ss=static_ss)==NULL)
      assert(0);
   while (ss->sm != savesm) {
      ss = ss->next;
      assert(ss != NULL);
   }
   if (ss->users != 1) {
      assert(ss->users>1);
      *sm = start_using_matrix(*savesm);
      unuse_matrix(savesm);
   }
}
