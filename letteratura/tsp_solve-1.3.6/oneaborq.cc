/* oneaborq.cc
 *
 * Improved the 1-Aborescence que element
 *
 * orig ceh
 */

#include <math.h>
#include "findtour.h"
#include "stdmacro.h"
#include "disperse.h"
#include "matrix.h"
#include <assert.h>
#include "io.h"
#include "params.h"
#include "oneaborq.h"
#include "rand.h"

#define SACBEST
/*
#define PRINT_IT
#define NODOTRAIN
#define CHECKTHECONS
#define SECBEST
#define USE_BRANCH_SECORD
 */
#define SYMMETRY
/*
#define HEUR
#define SLURR
#define SHARKIE
#define MINPRINT
#define SPECIAL1
#define SPECIAL2
#define SPECIAL0
 */
#define SPECIAL3
#define DECREASE

#define OPTIMAL optimal
/*
#define OPTIMAL ord.sum
 */

#define RAND(max) (((unsigned long)rand()/(double)MAX_RAND)*(max))
#define EDGERAND(max) ((((unsigned long)rand()/(double)MAX_RAND)+1.)*3.*max/4.)
#if defined(SPECIAL3) && !defined(SLURR)
# define SLURR
#endif

typedef struct chead_t {
   short id, head, trvd;
} chead_t;

#define MIN(I,J)  ( ((I) < (J)) ? (I) : (J) )

#ifdef HEUR
#define order(_a, _b) order(_a, 0)
#endif
#ifndef SECORD
#undef USE_BRANCH_SECORD
#endif

/* the ALPHA value
 *
 * they say to make a scalar from 1 to 2.
 * then they say start alpha at 2 and  periodically decrease by a factor till
 * it approaches zero.
 *
 * if DECREASE is not defined then alpha remains at START_ALPHA
 *
 * ALPHA FACTOR is the amount that alpha is decrease each time the langrangean
 * optimal stays the same from the last iteration.
 */

/* the ALPHA is one of the two lagrangean training multipliers, it is
 * said to be in the range from 0 to 2.  The following defines allow the
 * lamabda to start at START_ALPHA and decrease it through the training
 * until it ends at END_ALPHA when the passed number of geometric iterations
 * have been reached.  The ALPHA decreases at a geometric rate determined
 * by the ALPHA_FACTOR which is computed from the assumed number of iterations,
 * and the assumed start and end alpha values.  The alpha doesn't end at
 * END_ALPHA.  The end is determined by BREAKS #defines below.
 */
/*
#define START_ALPHA 2
#define ALPHA_FACTOR(iters) .1
 */
#define START_ALPHA 2
#define END_ALPHA .1
#define ALPHA_FACTOR(_start, _end, _iters) \
   pow((double)(_end)/(_start), (double)1./(_iters))

static float alpha;
static float alpha_factor;

/* The training is so important in finding a good tree to queue that we
 * don't want to stop iterations simply because we've reached a maximum number.
 * we now only stop training when we are satisfied we can't get much better
 * with another iteration.
 *
 * OPT_DIFF_THRESSHOLD is the amount of difference between trained optimals
 * that will break out of training if the difference does not break the
 * threshold.
 *
 * OPT_DIFF_BREAKS is the number of times the trained optimals must stay
 * with in the threshold to break out of training.
 */
/*
#define OPT_DIFF_THRESHOLD .5
 */
#define OPT_DIFF_THRESHOLD 1.0
/*
#define OPT_DIFF_BREAKS (17)
#define OPT_DIFF_BREAKS ((int)solver->matrix->degree/2)
 */
#define OPT_DIFF_BREAKS ((int)solver->matrix->degree*4)

/* This is the iterations of the initial ascent */
#define INITIAL_MAX_ITERATIONS ((int)(solver->matrix->degree*solver->matrix->degree)/50+16+solver->matrix->degree)
/*
#define INITIAL_MAX_ITERATIONS 4000
#define INITIAL_MAX_ITERATIONS 100000
 */
/* This is the iterations of the general ascent */
#define GENERAL_MAX_ITERATIONS ((int)(solver->matrix->degree-depth)/10+2)

#ifdef FLOAT_COST
# ifndef NDEBUG
#  define LCOST_TO_CITY(_cost) (assert((_cost) < 0 \
   && fequiv(_cost, (int)(_cost)-.5, .01)), (city_id_t)-(_cost))
# else
#  define LCOST_TO_CITY(_cost) ((city_id_t)-(_cost))
# endif
# define CITY_TO_LCOST(_city) (-((lambdacost_t)_city)-.5)
#else
# ifndef NDEBUG
#  define LCOST_TO_CITY(_cost) (assert((_cost) < 0), (city_id_t)-(_cost)-1)
# else
#  define LCOST_TO_CITY(_cost) ((city_id_t)-(_cost)-1)
# endif
# define CITY_TO_LCOST(_city) (-((lambdacost_t)_city)-1)
#endif

OneAbor :: ~OneAbor()
{
}

inline int OneAbor :: will_train(
#ifdef NODOTRAIN
   const sum_t)
{
   return 0;
#else
   const sum_t most)
{
   return (most - OPTIMAL) >
      (double)(.5/100.)*((most - solver->Initial_Lower_Bound));
   /*
   return 1;
    */
#endif
};

inline int OneAbor :: stat_will_train(const sum_t most)
{
   const double p = most - ord.sum;
   const double max = most - solver->Initial_Lower_Bound;
   short index = (short)((100.0*p)/max);

   if (index > 100 || index < 0) {
      if (index > 100) {
         assert(fabs(solver->Initial_Lower_Bound - ord.sum) <= SUM_GRANU);
         index = 100;
      }
      else
         index = 0;
      /*
      index = index > 100 ? 100 : 0;
       */
   }
   st_astats.dist[index]++;
   return will_train(most);
}

void OneAbor :: trip(
#ifndef PRINT_IT
   const char *)
{
#else
   const char *str)
{
if (param.verbose > 1) {
dump << str;
print();
}
#endif
}

class marker_t {
public:
   signed_city_id_t val;
   inline marker_t () { val = 0; };
   inline signed_city_id_t is_true() { return val; };
   inline signed_city_id_t is_false() { return (signed_city_id_t)!val; };
   inline void make_true() { val = 1; };
   inline void not() { val = (signed_city_id_t)!val; };
};

// first print the partial tours spearated by commas and print the
// cost it takes to travel from one site to another in the partial tours
// surrounded by '<' '>' characters.   Then print the one tree part
// as each city id and it's corresponding parent with the city who
// has no parent being a root of one of the possible one-trees, if the
// onetreee is complete it will only have one root.  finaly the last
// edge of the onetree is printed, the edge that separates the onetree from
// a minmums spaninng tree
void OneAbor :: print() const
{
   const city_a *city, *site;
   marker_t *marker;
   city_id_t id, save, last;

   dump.form("D%dO%.0lf ", (int)depth, (double)optimal);
   marker = new marker_t[solver->matrix->degree];
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (!city->is_section2())
         continue;
      if (marker[city->get_id(mic_list)].is_false()) {
         last = city->get_id(mic_list);
         id = city->u.sec2.required_edge;
         dump << city->get_id(mic_list) << "<";
         if (param.verbose > 2) {
#ifdef SYMMETRY
            dump << (long)(solver->matrix->val(city->get_id(mic_list), id)
             + city->lambda + mic_list[id].lambda);
#else
            if (city->section == 2)
               dump << (long)(solver->matrix->val(city->get_id(mic_list), id)
                + city->lambda);
            else
               dump << (long)(solver->matrix->val(id, city->get_id(mic_list))
                + mic_list[id].lambda);
#endif
         }
         dump << ">";
         marker[city->get_id(mic_list)].make_true();
         for (;;) {
            assert(marker[id].is_false());
            marker[id].make_true();
            site = mic_list+id;
            assert(site->section != 3);
            if (site->is_section2())
               break;
            save = last;
            last = id;
            id = site->up(save);
            dump << site->get_id(mic_list) << "<";
            if (param.verbose > 2)
               dump << (long)(solver->matrix->val(last, id)
                + mic_list[last].lambda
#ifdef SYMMETRY
                + mic_list[id].lambda
#endif
               );
            dump << ">";
         }
         assert(site->is_section2());
         dump << id << ",";
      }
   }
   dump << "*,";
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section == 1 && !marker[city->get_id(mic_list)].is_true())
         dump << "Missing Section 1 City " << city->get_id(mic_list) << "\n";
   }
   for (site = NULL, city = mic_list; city < mic_list.end_city; city++) {
      if (city->section == 1)
         continue;
      if (city->get_id(mic_list) == sacred_id)
         site = city;
      dump << city->get_id(mic_list) << "(";
      if (city->up() == NO_ID)
         dump << "RT";
      else {
         dump << "[" << city->up() << "]";
         if (param.verbose > 2)
            dump << (long)(solver->matrix->val(city->up(),
               city->get_id(mic_list)) + mic_list[city->up()].lambda
#ifdef SYMMETRY
               + city->lambda
#endif
               );
      }
      dump << "),";
   }
   if (site != NULL) {
      dump << site->get_id(mic_list) << "([" << sacred_edge << "]";
      if (param.verbose > 2) {
         if (sacred_edge == NO_ID)
            dump << "?";
         else
            dump << (long)(solver->matrix->val(sacred_edge,
               site->get_id(mic_list)) + mic_list[sacred_edge].lambda
#ifdef SYMMETRY
                + site->lambda
#endif
             );
      }
      dump << ")\n";
   }
   delete marker;
}

#ifdef CHECKTHECONS
/* Point *1*: ignore onetree subproblem check if it was meant to just crash and
 * die as an optimal MAX_SUM.
 */
void OneAbor :: check_connections()
{
   marker_t *marker;
   city_a *city;
   signed_city_id_t er = 0;
   city_id_t id, last, save;
   long num_branches = 0;

   if (optimal == MAX_SUM) /*1*/
      return;
   assert(sacred_id != NO_ID && sacred_edge != NO_ID);
   marker = new marker_t[solver->matrix->degree];
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section != 1)
         continue;
      marker[city->get_id(mic_list)].val = 2;
      num_branches += 2;
   }
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->is_section2()) {
         num_branches++;
         marker[city->get_id(mic_list)].val++;
         last = city->get_id(mic_list);
         id = city->u.sec2.required_edge;
         while (!mic_list[id].is_section2()) {
            assert(mic_list[id].section == 1);
            save = last;
            last = id;
            id = mic_list[id].up(save);
         }
         assert(city->u.sec2.end == id);
         assert(mic_list[id].u.sec2.end == city->get_id(mic_list));
      }
      if (city->section != 1 && city->up() != NO_ID) {
         num_branches += 2;
         marker[city->up()].val++;
         marker[city->get_id(mic_list)].val++;
      }
   }
   assert(num_branches == 2*solver->matrix->degree-2);
   marker[sacred_id].val++;
   marker[sacred_edge].val++;
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section != 1)
         continue;
      if (marker[city->get_id(mic_list)].val != 2) {
         dump << "([" << city->get_id(mic_list) << "]"
            << marker[city->get_id(mic_list)].val << "<>2) ";
         er = 1;
      }
      if (city->branches != 2) {
         dump << "([+" << city->get_id(mic_list) << "+]"
            << city->branches << "<>2) ";
         er = 1;
      }
   }
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section == 1)
         continue;
      if (marker[city->get_id(mic_list)].val != city->branches) {
         dump << "([" << city->get_id(mic_list) << "]"
            << marker[city->get_id(mic_list)].val << "<>"
            << city->branches << ") ";
         er=1;
      }
   }
   if (er) {
      dump << "\n";
      print();
   }
   delete marker;
}
#else
inline void OneAbor :: check_connections()
{
}
#endif

// C e H
inline void OneAbor :: operator = (const OneAbor &parent)
{
   depth = parent.depth;
   optimal = parent.optimal;
   lambda_sum = parent.lambda_sum;
   mic_list = parent.mic_list;
   ord = parent.ord;
   sacred_edge = parent.sacred_edge;
   sacred_id = parent.sacred_id;
   required_cost = parent.required_cost;
#ifdef SECORD
   sacred_secord = parent.sacred_secord;
#endif
}

void OneAbor :: heuristic_tour(Tour *tour, sum_t &opt)
{
   city_id_t *head = solver->xarray, *trvd = solver->arcj;
   city_id_t *id = solver->arci, x, y, root = NO_ID;
   city_a *city;
   CircList tourlist;
   TourListElement *t, *found_root=NULL;
   Path *p;
   TourIter ti;

   for (city = mic_list; city < mic_list.end_city; city++) {
      assert(city->section == 3);
      if (city->u.sec3.onetree_edge == NO_ID)
         root = city->get_id(mic_list);
      trvd[city->get_id(mic_list)] = 0;
      id[city->get_id(mic_list)]= city->u.sec3.onetree_edge;
      head[city->get_id(mic_list)] = (city_id_t) (city->branches != 2
         || city->get_id(mic_list) == sacred_edge
         || city->get_id(mic_list) == sacred_id);
   }
   assert(root != NO_ID);
   for (x = 0; x < solver->matrix->degree; x++) {
      if (head[x]) {
         assert(!trvd[x]);
         y = x;
         t = new TourListElement;
         tourlist.insert(t);
         trvd[y] = 1;
         t->tour.travel(y);
#ifdef SHARKIE
dump << y << ",";
#endif
         if (y != root)
         for (y = id[y]; !head[y]; y = id[y]) {
            if (y != root) {
               trvd[y] = 1;
               t->tour.travel(y);
#ifdef SHARKIE
dump << y << ",";
#endif
            }
            else { /* y==root */
               trvd[y] = 1;
               if (found_root == NULL) {
                  t->tour.travel(y);
#ifdef SHARKIE
dump << y << "*";
#endif
                  found_root = t;
               }
               else {
                  y = ((Path*)t->tour.get_head())->id;
#ifdef SHARKIE
dump << "\n";
#endif
                  ti.init(found_root->tour);
                  while ((p = ti.next()) != NULL) {
#ifdef SHARKIE
dump << p->id << "<";
#endif
                     t->tour.insert_after(NULL, p->id);
                  }
#ifdef SHARKIE
dump << "\n";
#endif
                  t->tour.change_head_to(y);
                  tourlist.del(found_root);
                  delete found_root;
               }
               break;
            }
         }
#ifdef SHARKIE
dump << "\n";
#endif
      }
   }
   DispersionHeuristic d(solver->matrix, tourlist);
   while (d.run())
      ;
   CircListIter ci(tourlist);
   while ((t = (TourListElement*)ci.next()) != NULL)
      delete t;
   if (solver->matrix->degree > 15) {
#ifdef SYMMETRY
      FindTour to("+ro+th+ro", solver->matrix, &solver->duration, 0, d.tour);
#else
      FindTour to("+k4+k3", solver->matrix, &solver->duration, 0, d.tour);
#endif
      if (param.verbose > 1)
         dump << "Disp=" << d.length()
            << " 3-Opt=" << to.tour->cost(solver->matrix) << "\n";
      if (to.tour->cost(solver->matrix) < opt) {
         opt = to.tour->cost(solver->matrix);
         tour->copy(*to.tour);
      }
   }
   else if (d.tour->cost(solver->matrix) < opt) {
      opt = d.tour->cost(solver->matrix);
      tour->copy(*d.tour);
   }
}

inline void OneAbor :: find_mins(const city_id_t x, const city_id_t t,
   const city_id_t v, const city_id_t m, const city_id_t n,
   const city_id_t bsize, const city_id_t *stkp,
   const city_id_t *stackp, lambdacost_t *bigm)
{
   const city_id_t *sp;
   lambdacost_t *bc, *be, best, best2, l;
   city_id_t i, j, s, oi;

   i = j = NO_ID;
   best2 = best = MAXLCOST;
   be = bigm + x;
   bc = bigm + x * bsize;
   for (sp = stkp; sp <= stackp; sp++) {
      s = *sp;
      assert(solver->minarray[s] != MAXLCOST);
      if (best > (l = be[s * bsize]) - solver->minarray[s]) {
         if (l != MAXLCOST) {
            best = l - solver->minarray[s];
            j = s;
         }
      }
      if (best2 > bc[s]) {
         best2 = bc[s];
         i = s;
      }
   }
   if (best != MAXLCOST) {
      bigm[x + m * bsize] = best;
      assert(j != NO_ID);
      if (j >= n) {
         assert(solver->shadow[solver->line[j]] != NO_ID);
         j = LCOST_TO_CITY(bigm[solver->line[x]
          + bsize * solver->shadow[solver->line[j]]]);
      }
   }
   else
      assert(bigm[x + m * bsize] == MAXLCOST);
   if (best2 != MAXLCOST) {
      bigm[m + x * bsize] = best2;
      assert(i != NO_ID);
      if (i >= n) {
         assert(solver->shadow[solver->line[i]] != NO_ID);
         oi = i = LCOST_TO_CITY(bigm[solver->shadow[solver->line[i]]
          + bsize * solver->line[x]]);
      }
         oi = i;
      /*
      else
         oi = mic_list[i].head(mic_list);
       */
   }
   else{
      oi = NO_ID;
      assert(bigm[m + x * bsize] == MAXLCOST);
   }
   s = solver->line[x];
   bigm[s + bsize * v] = CITY_TO_LCOST(j);
   bigm[v + bsize * s] = CITY_TO_LCOST(oi);
   if (x >= n) {
      s = solver->shadow[s];
      assert(s != NO_ID);
      if (i != NO_ID)
         bigm[t + bsize * s] = bigm[i + bsize * s];
      if (j != NO_ID)
         bigm[s + bsize * t] = bigm[s + bsize * j];
   }
}

int OneAbor :: construct_tour(Tour *tour)
{
   city_id_t last_id, id, save, count = solver->matrix->degree, sedge = NO_ID;
   long state, section2s;
   city_a *city;

   trip("CNS ");
   for (city = mic_list; city < mic_list.end_city; city++)
      if (city->branches == 1 && city->get_id(mic_list) != sacred_id)
         sedge = city->get_id(mic_list);
   if (c_factor() == 0) {
      if (sedge != NO_ID) {
         if (param.verbose)
            dump << __FILE__ << ":" << __LINE__ << "\n";
         return 0;
      }
      sedge = sacred_edge;
   }
   if (c_factor() != 0) {
      dump << "Bad C FACTOR SEED=" << (param.seed-1) << "\n";
      return 0;
   }
   state = 1;
   section2s = 0;
   last_id = NO_ID;
   city = mic_list+sacred_id;
   id = city->get_id(mic_list);
   for(; count > 0; city = mic_list+id, count--) {
      if (state)
         tour->travel(id);
      else
         tour->insert_after(NULL, id);
      switch (city->section) {
      case 3:
         if (city->u.sec3.onetree_edge == NO_ID) {
            state--;
            if (state) {
               if (param.verbose)
                  dump << __FILE__ << ":" << __LINE__ << "\n";
               return 0;
            }
            last_id = sacred_id;
            id = sedge;
         }
         else {
            last_id = id;
            id = city->u.sec3.onetree_edge;
         }
         break;
      case 1:
         save = last_id;
         last_id = id;
         id = city->up(save);
         break;
      default:
         if (!city->is_section2()) {
            if (param.verbose)
               dump << __FILE__ << ":" << __LINE__ << "\n";
            return 0;
         }
         if ((section2s++) % 2) {
            last_id = id;
            id = city->u.sec2.onetree_edge;
         }
         else {
            last_id = id;
            id = city->u.sec2.required_edge;
         }
         if (id == NO_ID) {
            state--;
            if (state) {
               if (param.verbose)
                  dump << __FILE__ << ":" << __LINE__ << "\n";
               return 0;
            }
            last_id = sacred_id;
            id = sedge;
         }
         break;
      }
      if (id == NO_ID) {
         if (param.verbose)
            dump << __FILE__ << ":" << __LINE__ << "\n";
         return 0;
      }
   }
   if (solver->matrix->degree > 10) {
#ifdef SYMMETRY
      FindTour to("+ro+th+ro", solver->matrix, &solver->duration, 0, tour);
#else
      FindTour to("+k4+k3", solver->matrix, &solver->duration, 0, tour);
#endif
      tour->copy(*to.tour);
   }
   optimal = tour->cost(solver->matrix);
   return 1;
}

/* Point *1*: The lambdas associated with the micmap already traveled don't
 * affect lambda training, but do get lambda addition.
 *
 */
inline void OneAbor :: sum_lambdas()
{
   city_a *city;

   lambda_sum = (lambdasum_t)0;
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section == 1)
         continue;
      lambda_sum += city->lambda*(lambdasum_t)((int)city->branches-2);
   }
   optimal += (sum_t)lambda_sum;
}

// For SYMMETRY we first set an array of ids to NO_ID, then fill the array
// with down pointers, while at the same time looking at all the 1 branch
// vertecies and finding the largest edge from them.  Then for the largest
// 1 branch vertex, we find the second lowest cost (since the first would
// obviously be the 1 branch.
//
// Point *1*: we don't use city->lambda here since i could change
//
// Point *2*: now check the cost of the root city if it has 1 branch
//
// Point *3*: pick the root with one branch if possible
//
// Point *4*: needs to set the a_up correctly if the origonal is still sacred_id
//
// Point *6*: don't include sacred_id lambda cause it is not compared with it
//
// Point *7*: even if sacred edge is forbidden, we will want to check the
// origonal sacred_id, and use as cost the cost of the forbidden sacred id.
//
// For ASYMMETRY
// recalculate/find the sacred edge to the onetree
// this can be called anytime that the sacred edge is not connected.
// this finds the shortest edge from the root of the tree to another vertex.
//
// FIX: the sacred_id is found here but it could be bookept with each OneAbor
// node, since you will always know the root of the minspan tree and that will
// always be the sacred_id.
//
void OneAbor :: sacred_span()
{
   lambdacost_t max_cost, l;
   city_a *city, *sacred_city;
   city_id_t i;

#ifdef SYMMETRY
   city_id_t root, *down = solver->xarray, a_up;

# ifdef SACBEST
   city_id_t s, ss, ii;
   cost_t *co;
   lambdacost_t min_cost;

   sacred_id = sacred_edge = ss = ii = root = NO_ID; //OPTWarn
#ifndef NDEBUG
   for (i = 0; i < solver->matrix->degree; i++)
      down[i] = NO_ID;
#endif NDEBUG
   sacred_city = mic_list;
   max_cost = MINLCOST;
   for (s = 0; sacred_city < mic_list.end_city; sacred_city++, s++) {
      if (sacred_city->section == 1)
         continue;
      co = solver->matrix->cost[s];
      if ((a_up = sacred_city->up()) != NO_ID) {
         down[a_up] = sacred_city->get_id(mic_list);
         if (mic_list[a_up].is_section2())
            down[mic_list[a_up].u.sec2.end] = sacred_city->get_id(mic_list);
      }
      if ((a_up = sacred_city->up_root(mic_list)) == NO_ID) {
         if (root == NO_ID || sacred_city->branches == 1) /*3*/
            root = s;
      }
      else if (sacred_city->branches == 1) {
         min_cost = MAXLCOST;
         if (sacred_city->is_section2()) {
            a_up = sacred_city->u.sec2.end;
         }
         for (i = 0, city = mic_list; city < mic_list.end_city; city++, i++) {
            if (min_cost > (l = co[i] + city->lambda) && city->section != 1
             && i != s && i != a_up && city->fs.not_forbidden(s)) {
               min_cost = l;
               ss = s;
               ii = i;
            }
         }
         if (min_cost != MAXLCOST
          && max_cost < (l = min_cost + sacred_city->lambda)) {
            max_cost = l;
            sacred_id = ss;
            sacred_edge = ii;
         }
      }
   }
   assert(root != NO_ID);
   if (mic_list[root].branches == 1) {
      if (down[root] == NO_ID) {
         assert(mic_list[root].is_section2());
         sacred_id = root;
         sacred_edge = mic_list[root].u.sec2.end;
         goto sacred_end;
      }
      co = solver->matrix->cost[root];
      min_cost = MAXLCOST;
      if (mic_list[root].is_section2())
         a_up = mic_list[root].u.sec2.end;
      else
         a_up = NO_ID;
      for (i = 0, city = mic_list; city < mic_list.end_city; city++, i++) {
         if (min_cost > (l = co[i] + city->lambda) && city->section != 1
          && i != root && i != a_up && city->fs.not_forbidden(root)) {
            min_cost = l;
            ss = root;
            ii = i;
         }
      }
      if (max_cost <= (l = min_cost + sacred_city->lambda)) {
         max_cost = l;
         sacred_id = ss;
         sacred_edge = ii;
      }
   }
   if (sacred_edge == NO_ID) {
      optimal = MAX_SUM;
      return;
   }
#  ifndef NDEBUG
   else
      assert(down[root] != NO_ID);
#  endif
   
# else
   lambdacost_t mc;

#  ifdef SECORD
   sacred_secord = MAXLCOST;
#  endif
   if (sacred_id != NO_ID
    && (sacred_city = mic_list+sacred_id)->section != 1
    && sacred_city->branches == 1) {
      max_cost = solver->matrix->val(sacred_id, sacred_edge)
       + sacred_city->lambda + mic_list[sacred_edge].lambda; /*7*/
   }
   else {
      max_cost = MINLCOST;
      sacred_city = NULL;
   }
   a_up = root = NO_ID;
   for (i = 0; i < solver->matrix->degree; i++)
      down[i] = NO_ID;
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section == 1)
         continue;
      i = city->get_id(mic_list);
      if (city->up_root(mic_list) == NO_ID) {
         if (root == NO_ID || city->branches == 1) /*3*/
            root = i;
         continue;
      }
      a_up = city->up();
      if (a_up != NO_ID) {
         down[a_up] = i;
         if (mic_list[a_up].is_section2())
            down[mic_list[a_up].u.sec2.end] = i;
      }
      if (city->branches != 1)
         continue;
      if (a_up == NO_ID) {
         assert(city->is_section2());
         a_up = city->u.sec2.required_edge;
      }
      if (max_cost < (l = solver->matrix->val(i, a_up)
       + city->lambda + mic_list[a_up].lambda)) { /*1*/
         max_cost = l;
         sacred_city = city;
      }
   }
   assert(root != NO_ID);
   if (mic_list[root].branches == 1) { /*2*/
      i = down[root];
      if (i == NO_ID) {
         assert(mic_list[root].is_section2());
         sacred_id = root;
         sacred_edge = mic_list[root].u.sec2.end;
         goto sacred_end;
      }
      city = mic_list + root;
      assert(sacred_city != NULL);
      if ((a_up = mic_list[i].up_root(mic_list)) != root) {
         assert(city->is_section2() && a_up == city->u.sec2.end);
         a_up = root;
         i = city->u.sec2.required_edge;
      }
      if (max_cost < solver->matrix->val(i, a_up)
       + mic_list[i].lambda + mic_list[a_up].lambda
       || sacred_city == mic_list+root) { /*4*/
         sacred_city = mic_list+root;
         a_up = sacred_city->is_section2() ? NO_ID : i;
      }
      else {
         a_up = sacred_city->up();
      }
   }
   else {
      assert(sacred_city != NULL);
      a_up = sacred_city->up();
   }
   mc = max_cost = MAXLCOST;
   sacred_id = sacred_city->get_id(mic_list);
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section == 1 || sacred_id == (i = city->get_id(mic_list))
       || i == a_up || (city->is_section2()
       ? (city->u.sec2.end == sacred_id) : 0))
         continue;
      if (max_cost > (l = solver->matrix->val(i, sacred_id) + city->lambda)
       && sacred_city->fs.not_forbidden(i)) {
         mc = max_cost;
         max_cost = l;
         sacred_edge = i;
      }
      else if (mc > l) {
         mc = l;
      }
   }
   if (sacred_edge == NO_ID) {
      optimal = MAX_SUM;
      return;
   }
#  ifdef SECORD
   if (mc != MAXLCOST)
      sacred_secord = mc - max_cost;
#  endif
# endif // SACBEST
   sacred_end:
   assert(sacred_id != NO_ID && sacred_edge != NO_ID);
#else // SYMMETRY
   if (sacred_edge == NO_ID) {
      sacred_city = mic_list + sacred_id;
      max_cost = MAXLCOST;
      for (city = mic_list; city < mic_list.end_city; city++) {
         if (city->section == 1 || city->section == 2 || sacred_city == city
          || (city->is_section2() ? sacred_id == city->u.sec2.end : 0))
            continue;
         i = city->get_id(mic_list);
         if (max_cost > (l = solver->matrix->val(i, sacred_id) + city->lambda)
          && sacred_city->fs.not_forbidden(i)) {
            max_cost = l;
            sacred_edge = i;
         }
      }
      if (max_cost == MAXLCOST) {
         optimal = MAX_SUM;
         return;
      }
   }
#endif // SYMMETRY
   mic_list[sacred_id].branches++;
   mic_list[sacred_edge].branches++;
   optimal += solver->matrix->val(sacred_edge, sacred_id);
   sum_lambdas();
}

inline void OneAbor :: check_minarray(
#ifndef SECORD
   lambdacost_t &,
   city_a *, const city_a *, const city_id_t, const city_id_t,
   cost_t *, city_id_t &, city_id_t &)
{
#else
   lambdacost_t &minarray,
   city_a *city, const city_a *scity, const city_id_t y, const city_id_t x,
   cost_t *cost, city_id_t &onetree_edge, city_id_t &sonetree_edge)
{
   lambdacost_t l;
   if (city->secord_val > (l = (lambdacost_t)cost[y]
    + mic_list[x].lambda + scity->lambda) && scity->fs.not_forbidden(x)) {
      if (minarray > l) {
         city->secord_val = minarray;
         minarray = l;
         sonetree_edge = onetree_edge;
         onetree_edge = scity->get_id(mic_list);
      }
      else {
         city->secord_val = l;
         sonetree_edge = scity->get_id(mic_list);
      }
   }
#endif
}

inline int OneAbor :: same_colors_as_children(
   const city_id_t check_color, const city_id_t *elist)
{
   const city_id_t *listptr = (const city_id_t *)solver->arci;
   const city_id_t *slinr = elist;
   const city_id_t *color = (const city_id_t *)solver->parent;

   do {
      if (check_color == color[*slinr])
         return 1;
      slinr = listptr + *slinr;
   } while (slinr != elist);
   return 0;
}

/* Point *3*: Find New secord cause the one i'm pointing to goes to another
 * child.
 *
 * Point *4*: mark the parent color as the child color so it won't get the best
 * edge. and mark it back when done.
 *
 * Point *5*: finally color this child's tree the parent color
 */
inline void OneAbor :: change_colors_of_children(
#ifndef SECORD
   lambdacost_t &, city_id_t &, const city_id_t, const city_id_t)
{
#else
   lambdacost_t &best_secord, city_id_t &best_seccity,
   const city_id_t parrentx, const city_id_t parrentxend)
{
   city_id_t *listptr = solver->arci, *seccity = solver->arcj;
   city_id_t *color = solver->parent, *elist, *linr, *slinr, *elin;
   city_id_t colorx = color[parrentx], y, xend, x, lin;
   city_id_t child_seccity, child_color;
   lambdacost_t childbest, l;
   city_a *scity;
   cost_t *cost, *ec = NULL;

   best_seccity = NO_ID;
   best_secord = MAXLCOST;
   elin = color + solver->matrix->degree;
   elist = listptr + parrentx;
   linr = listptr + (lin = *elist);
   if (parrentxend != NO_ID)
      color[parrentxend] = colorx;
   for (; linr != elist; linr = listptr + (lin = *linr)) {
      child_color = color[lin];
      if (seccity[lin] != NO_ID
       && same_colors_as_children(color[seccity[lin]], elist)) { /*3*/
         color[parrentx] = child_color; /*4*/
         if (parrentxend != NO_ID) {
            /*
            assert(colorx == color[parrentxend]);
             */
            color[parrentxend] = child_color;
         }
#ifndef NDEBUG
         childbest = MAXLCOST; /* for assertions below loop */
         child_seccity = NO_ID;
#else
         childbest = best_secord;
         child_seccity = best_seccity;
#endif
         for (slinr = color; slinr < elin; slinr++) {
            if (*slinr != child_color
             || ((x = (slinr - color)) == parrentx) || x == parrentxend)
               continue;
            scity = mic_list + x;
            if (scity->section == 2) {
               xend = scity->u.sec2.end;
               ec = solver->matrix->cost[xend];
            }
            else
               xend = NO_ID;
            cost = solver->matrix->cost[x];
            for (scity = mic_list; scity < mic_list.end_city; scity++) {
               if (scity->section == 1
                || color[y = scity->get_id(mic_list)] == child_color)
                  continue;
               if (childbest
                > (l = (lambdacost_t)cost[scity->get_id(mic_list)]
                + mic_list[x].lambda + scity->lambda)
                && scity->fs.not_forbidden(x)
                && !same_colors_as_children(color[y], elist)) {
                  childbest = l;
                  child_seccity = scity->get_id(mic_list);
               }
               if (xend != NO_ID) {
                  if (childbest
                   > (l = (lambdacost_t)ec[scity->get_id(mic_list)]
                   + mic_list[xend].lambda + scity->lambda)
                   && scity->fs.not_forbidden(xend)
                   && !same_colors_as_children(color[y], elist)) {
                     childbest = l;
                     child_seccity = scity->get_id(mic_list);
                  }
               }
            }
         }
         color[parrentx] = colorx; /*4*/
         if (parrentxend != NO_ID)
            color[parrentxend] = colorx;
         assert((child_seccity == NO_ID) ? 1 : (seccity[lin] != child_seccity));
         assert(mic_list[lin].secord_val <= childbest);
         if (best_secord > childbest) {
            best_secord = childbest;
            best_seccity = child_seccity;
         }
         assert (!(child_seccity != NO_ID
          && same_colors_as_children(color[child_seccity], elist)));
      }
      else if (best_secord > mic_list[lin].secord_val) {
         best_secord = mic_list[lin].secord_val;
         best_seccity = seccity[lin];
      }
      if (listptr[*elist] == parrentx) { /*only 1color*/
         assert(elist-listptr == parrentx);
         if (parrentxend != NO_ID)
            color[parrentxend] = child_color;
         color[parrentx] = child_color;
      }
      else {
         for (slinr = color; slinr < elin; slinr++) { /*5*/
            if (*slinr == child_color)
                *slinr = colorx;
         }
      }
   }
#endif
}

/* Point *1*: chosen for next step if label is not NO_ID and if the branches
 * have been full filled.
 *
 * Point *2*: make sure the child is in the parrents list
 */
void OneAbor :: compute_secords()
{
#ifdef SECORD
   signed_city_id_t *bch = (signed_city_id_t*)solver->xarray;
   city_id_t *label = solver->label, xend;
   city_id_t *color = solver->parent, x, lin, y, onetree_edge;
   city_id_t *listptr = solver->arci, *secord = solver->arcj, *elin;
   lambdacost_t *minarray = solver->minarray;
   city_a *city, *scity;
   cost_t *cost, *ec = NULL;
   
   mic_list[sacred_id].branches--;
   mic_list[sacred_edge].branches--;
   elin = color + solver->matrix->degree;
   for (city = mic_list; city < mic_list.end_city; city++) {
      x = city->get_id(mic_list);
      color[x] = (city->section == 4) ? city->u.sec2.end : x;
      /*
      color[x] = x;
       */
      listptr[x] = x;
      if (city->section == 2)
         bch[x] = city->branches
          - (city->branches + mic_list[city->u.sec2.end].branches
          - ((city->up_root(mic_list) == NO_ID) ? 2 : 3));
      else if (city->up_root(mic_list) == NO_ID)
         bch[x] = 0;
      else
         bch[x] = 1;
      label[x] = (city->branches == bch[x]) ? 1 : 0;
      city->secord_val = MAXLCOST;
   }
   for (city = mic_list; ; city++) {
      if (city >= mic_list.end_city)
         city = mic_list;
      x = city->get_id(mic_list);
      if (city->section != 1 && city->section != 4
       && label[x] && bch[x] == city->branches) { /*1*/
         if (city->up_root(mic_list) == NO_ID) 
            break;
         if (city->section == 2) {
            xend = city->u.sec2.end;
            ec = solver->matrix->cost[xend];
         }
         else
            xend = NO_ID;
         if ((y = listptr[x]) != x) {
            change_colors_of_children(minarray[x], onetree_edge, x, xend);
            secord[x] = NO_ID;
#ifndef NDEBUG
            listptr[x] = x;
#endif
         }
         else {
            onetree_edge = secord[x] = NO_ID;
            minarray[x] = MAXLCOST;
            if (xend != NO_ID)
               color[xend] = color[x];
         }
         label[x] = 0;
         cost = solver->matrix->cost[x];
         lin = color[x];
         for (scity = mic_list; scity < mic_list.end_city; scity++) {
            if (scity->section != 1
              && color[y = scity->get_id(mic_list)] != lin) {
               check_minarray(minarray[x], city, scity, y, x, cost,
                  onetree_edge, secord[x]);
               if (xend != NO_ID)
                  check_minarray(minarray[x], city, scity, y, xend, ec,
                     onetree_edge, secord[x]);
            }
         }
         y = city->up_root(mic_list);
         assert(y == onetree_edge);
         if (mic_list[y].section == 4)
            y = mic_list[y].u.sec2.end;
         bch[y]++;
         label[y] = 1;
         assert(listptr[x] == x);
         listptr[x] = listptr[y]; /*2*/
         listptr[y] = x;
      }
   }
#if defined(USE_BRANCH_SECORD)
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section != 1 && city->up() != NO_ID) {
         if (city->section == 4) {
            if (mic_list[city->u.sec2.end].secord_val != MAXLCOST) 
               mic_list[city->u.sec2.end].secord_val
                -= solver->matrix->val(city->up(), city->get_id(mic_list))
                + city->lambda + mic_list[city->up()].lambda;
            city->secord_val = mic_list[city->u.sec2.end].secord_val;
         }
         else if (city->secord_val != MAXLCOST) {
            city->secord_val
             -= solver->matrix->val(city->up(), city->get_id(mic_list))
             + city->lambda + mic_list[city->up()].lambda;
            if (city->section == 2)
               mic_list[city->u.sec2.end].secord_val = city->secord_val;
         }
         assert(city->secord_val >= 0.);
      }
   }
#endif
   mic_list[sacred_id].branches++;
   mic_list[sacred_edge].branches++;
#endif
}

// compute the minimum spanning tree assuming you have forced edges
// in sections 1 and 2, and then compute the sacred onetree edge to
// make it a complete relaxation
//
// Point *1*: we call sacred_span and this adds to the lambda_sum but
// the lambda sum is overwritten in sum_lambdas();
//
// Point *2*: we make the xarray list of cities that can be traveled to.
//
// Point *3*: if section2 city is attempted, assign the not_x to be it's twin
//
// Point *4*: Make the section4 city right after the section2 city.
//
// Point *5*: disallow onetree edges from going to a section 4 city.
//
// Point *6*: look through cycle for largest edge and extract.
//
// Point *7*: point up the oposite way the cost goes
//
void OneAbor :: min_span()
{
   city_id_t start = NO_ID, x = 0, j, xend;
   lambdacost_t best;
   city_a *city, *scity;
   city_id_t *startp;
   city_id_t *xarray = solver->xarray, *xp;
   lambdacost_t *minarray = solver->minarray;
   static gcount = 0;
   gcount++;
   gc = gcount;
#ifdef SYMMETRY
   city_id_t not_x, *endp = NULL;
#ifdef SECBEST
   lambdacost_t *secbest = solver->bigm, *firbest = solver->loc2;
#endif
   lambdacost_t l;
#else
   city_id_t bsize = solver->matrix->degree*2 - 2, stage, m, t = NO_ID, i, v, n;
   lambdacost_t *bc, *be, *bigm = solver->bigm, *bs;
   void *vod;
   city_id_t *label = solver->label, *parent = solver->parent, *stackp, *sp;
   city_id_t *arci = solver->arci, *arcj = solver->arcj, *stack = solver->stack;
   city_id_t *line = solver->line, *shadow = solver->shadow, *stkp;

   stackp = stack;
#endif

   optimal = required_cost;
#ifdef PRINT_IT
   if (param.verbose > 3)
      solver->matrix->print();
#endif
   xp = &x;
   startp = NULL;
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section != 1) {
         city->reset();
#ifdef SECBEST
         j = city->get_id(mic_list);
         secbest[j] = firbest[j] = MAXLCOST;
         if (city->section != 4) { /*2*/
#else
         if (city->section != 4) { /*2*/
            j = city->get_id(mic_list);
#endif
            startp = xp;
            xp = xarray + (*xp = j);
#ifdef SYMMETRY
            minarray[j] = MAXLCOST;
         }
#else
            line[j] = j;
# ifndef NDEBUG
            minarray[j] = MAXLCOST;
# endif
            shadow[j] = label[j] = parent[j] = NO_ID;
            bc = bigm + j * bsize;
            for (scity = mic_list; scity < mic_list.end_city; bc++, scity++) {
               switch (scity->section) {
               case 3:
               case 2:
                  if (scity->get_id(mic_list) != j) {
                     xend = scity->head(mic_list);
                     *bc = solver->matrix->cost[xend][j]
                      + mic_list[xend].lambda;
                     break;
                  }
                  /*
                  goto swdef;
                  if (scity->tail(mic_list) != j) {
                     break;
                  }
               swdef:
                */
               default:
                  *bc = MAXLCOST;
                  break;
               }
            }
            bc = bigm + j * bsize;
            for (vod = NULL; (xend = city->fs.iter(vod)) != NO_ID; ) {
               scity = mic_list + xend;
               if (scity->section == 3)
                  bc[xend] = MAXLCOST;
               else if (scity->section == 4)
                  bc[scity->tail(mic_list)] = MAXLCOST;
            }
         }
         else {
            j = city->get_id(mic_list);
            parent[j] = NO_ID;
            /*
            parent[j] = MAX_DEGREE;
             */
            line[j] = city->tail(mic_list);
            bc = bigm + bsize * j;
            be = bc + solver->matrix->degree;
            while (bc < be)
               *bc++ = MAXLCOST; // FIX: CAN JUST NOT LOOK AT HEADS below...
         }
#endif
      }
   }
   if (startp == &x)
      startp = xp;
   *xp = x;
   assert(startp != NULL);
   start = *startp;
#ifdef SYMMETRY
# ifdef __BORLANDC__
   while (xp = startp, (*startp = xarray[start]) != start) {
# else
   while ((*(xp = startp) = xarray[start]) != start) {
# endif
      best = MAXLCOST;
# ifndef NDEBUG
      endp = NULL;
# endif
      if (mic_list[start].section == 2) /*3*/
         not_x = mic_list[start].u.sec2.end;
      else
         not_x = NO_ID;
      do {
         scity = city = mic_list + (x = *xp);
         j = NO_ID;
# ifdef SECBEST
         if (secbest[x] > (l = (lambdacost_t)solver->matrix->val(start, x)
          + city->lambda + mic_list[start].lambda)
          && city->fs.not_forbidden(start)) {
            j = start;
            if (minarray[x] > l)
               minarray[x] = l;
            if (firbest[x] > l) {
               secbest[x] = firbest[x];
               firbest[x] = l;
            }
            else
               secbest[x] = l;
         }
         if (not_x != NO_ID && secbest[x] > (l = (lambdacost_t)
          solver->matrix->val(not_x, x) + city->lambda + mic_list[not_x].lambda)
          && city->fs.not_forbidden(not_x)) {
            j = not_x;
            if (minarray[x] > l)
               minarray[x] = l;
            if (firbest[x] > l) {
               secbest[x] = firbest[x];
               firbest[x] = l;
            }
            else
               secbest[x] = l;
         }
         if (city->section == 2) {
            city = mic_list + (xend = city->u.sec2.end);
            if (secbest[x] > (l = (lambdacost_t)
             solver->matrix->val(start, xend) + city->lambda
             + mic_list[start].lambda) && city->fs.not_forbidden(start)) {
               j = start;
               scity = city;
               if (minarray[x] > l)
                  minarray[x] = l;
               if (firbest[xend] > l) {
                  secbest[xend] = firbest[xend];
                  firbest[xend] = l;
               }
               else
                  secbest[xend] = l;
            }
            if (not_x != NO_ID && secbest[x] > (l = (lambdacost_t)
             solver->matrix->val(not_x, xend) + city->lambda
             + mic_list[not_x].lambda) && city->fs.not_forbidden(not_x)) {
               j = not_x;
               scity = city;
               if (minarray[x] > l)
                  minarray[x] = l;
               if (firbest[xend] > l) {
                  secbest[xend] = firbest[xend];
                  firbest[xend] = l;
               }
               else
                  secbest[xend] = l;
            }
         }
# else // SECBEST
         if (minarray[x] > (l = (lambdacost_t)solver->matrix->val(start, x)
          + city->lambda + mic_list[start].lambda)
          && city->fs.not_forbidden(start)) {
            j = start;
            minarray[x] = l;
         }
         if (not_x != NO_ID && minarray[x] > (l = (lambdacost_t)
          solver->matrix->val(not_x, x) + city->lambda + mic_list[not_x].lambda)
          && city->fs.not_forbidden(not_x)) {
            j = not_x;
            minarray[x] = l;
         }
         if (city->section == 2) {
            city = mic_list + (xend = city->u.sec2.end);
            if (minarray[x] > (l = (lambdacost_t)
             solver->matrix->val(start, xend) + city->lambda
             + mic_list[start].lambda) && city->fs.not_forbidden(start)) {
               j = start;
               minarray[x] = l;
               scity = city;
            }
            if (not_x != NO_ID && minarray[x] > (l = (lambdacost_t)
             solver->matrix->val(not_x, xend) + city->lambda
             + mic_list[not_x].lambda) && city->fs.not_forbidden(not_x)) {
               j = not_x;
               minarray[x] = l;
               scity = city;
            }
         }
# endif // SECBEST
         if (j != NO_ID) {
            if (scity->section == 3)
               scity->u.sec3.onetree_edge = j;
            else {
               scity->u.sec2.onetree_edge = j;
               mic_list[scity->u.sec2.end].u.sec2.onetree_edge = NO_ID;
            }
         }
# ifdef SECBEST
         if (secbest
         if (best > (l = minarray[x])) {
            endp = xp;
            best = l;
         }
# else // SECBEST
         if (best > minarray[x]) {
            endp = xp;
            best = minarray[x];
         }
# endif // SECBEST
      } while ((xp = (xarray+x)) != startp);
      if (best == MAXLCOST) {
         optimal = MAX_SUM;
         return;
      }
      assert(endp != NULL);
      start = *(startp = endp);
   }
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section != 1 && city->up() != NO_ID) {
         optimal += solver->matrix->val(city->up(), city->get_id(mic_list));
         city->branches++;
         mic_list[city->up()].branches++;
      }
   }
#else // SYMMETRY
   m = (n = solver->matrix->degree) - 1;
   assert(MAX_DEGREE > n*2 - 2);
   j = NO_ID;
   label[start] = stage = 0;
   minarray[sacred_id = start] = 0;
   for (;;) {
      xp = startp;
      do {
         if (label[v = *xp] == NO_ID)
            break;
      } while ((xp = xarray + v) != startp);
      if (xp == startp)
         break;
      stage++;
      *(stackp = stack) = v;
      for (;;) {
         best = MAXLCOST;
         for (bs = bc = bigm + v * bsize, be = bc + m; bc <= be; bc++) {
            if (best > *bc && parent[(city_id_t)(bc - bs)] == NO_ID) {
               best = *bc;
               assert(best > 1e-10 || best <= 0);
               t = (city_id_t)(bc - bs);
            }
         }
         if (best == MAXLCOST) {
            optimal = MAX_SUM;
            return;
         }
         if (t < n) {
            assert(shadow[line[t]] == NO_ID);
            i = t;
            /*
            if (mic_list[t].section == 4)
               t = mic_list[t].tail(mic_list);
             */
            assert(mic_list[t].section != 4);
         }
         else {
            assert(shadow[line[t]] != NO_ID);
            i = LCOST_TO_CITY(bigm[shadow[line[t]] + bsize * line[v]]);
         }
         if (v < n) {
            assert(shadow[line[v]] == NO_ID);
            j = v;
            assert(mic_list[v].section != 4);
         }
         else {
            assert(shadow[line[v]] != NO_ID);
            j = LCOST_TO_CITY(bigm[line[t] + bsize * shadow[line[v]]]);
         }
         label[v] = stage;
         arci[v] = i;
         arcj[v] = j;
         minarray[v] = best;
         if (label[t] == NO_ID) {
            v = *(++stackp) = t;
         }
         else if (label[t] == stage) {
            m++;
            assert(m < bsize);
            label[m] = parent[m] = NO_ID;
            for (bc = bigm + m * bsize, be = bc + m-1; bc <= be; bc++)
               *bc = MAXLCOST;
            for (bc = bigm + m, be = bc + bsize * m; bc <= be; bc += bsize)
               *bc = MAXLCOST;
            for (stkp = stack; *stkp != t; stkp++) {
               x = *stkp;
               assert(label[*stkp] != NO_ID);
               label[x]++;
               /*
               if (x < n && (scity = mic_list+x)->is_section2())
                  label[scity->u.sec2.end]++;
                   */
               assert(stkp != stackp);
               assert(minarray[x] != MAXLCOST);
            }
            for (sp = stkp; sp <= stackp; sp++) {
               parent[x = *sp] = m;
               assert(minarray[x] != MAXLCOST);
               /*
               if (x < n && (scity = mic_list+x)->is_section2())
                  parent[scity->u.sec2.end] = m;
                   */
            }
            xp = startp;
            do {
               x = *xp;
               assert ((label[x] == stage) ? parent[x] != NO_ID : 1);
               if (parent[x] != NO_ID)
                  continue;
               find_mins(x, t, v, m, n, bsize, stkp, stackp, bigm);
            } while ((xp = xarray + x) != startp);
            for (x = n; x < m; x++) {
               if (label[x] == stage)
                  continue;
               find_mins(x, t, v, m, n, bsize, stkp, stackp, bigm);
            }
            line[m] = t;
            shadow[t] = v;
            for (xp = stack; *xp != t; xp++) {
               label[x = *xp]--;
               /*
               if (x < n && (scity = mic_list+x)->is_section2())
                  label[scity->u.sec2.end]++;
                   */
               assert(label[x] == stage);
            }
            v = *(stackp = stkp) = m;
         }
         else
            break;
      }
   }
   for (x = m; x >= n; x--) {
      if (label[x] != NO_ID) {
         city = mic_list + (j = arcj[x]);
         assert (j < n);
         if (j == NO_ID || (i = arci[x]) == NO_ID) {
            optimal = MAX_SUM;
            return;
         }
         city->upset() = i = mic_list[arci[x]].head(mic_list);
         optimal += solver->matrix->val(i, j);
         city->branches++;
         mic_list[i].branches++;
         assert(city->section != 4 && city->section != 1);
         assert(mic_list[arci[x]].section != 1);
         assert(mic_list[arci[x]].section != 4);
         while (j != x) {
            label[j] = NO_ID;
            j = parent[j];
         }
      }
   }
   label[sacred_id] = NO_ID;
   xp = startp;
   do {
      if (label[x = *xp] != NO_ID) {
         city = mic_list + (j = arcj[x]);
         city->upset() = i = mic_list[arci[x]].head(mic_list);
         optimal += solver->matrix->val(i, j);
         city->branches++;
         mic_list[i].branches++;
         assert(city->section != 4 && city->section != 1);
         assert(mic_list[arci[x]].section != 1);
         assert(mic_list[arci[x]].section != 4);
         while (j != x) {
            label[j] = NO_ID;
            j = parent[j];
         }
      }
   } while ((xp = xarray + x) != startp);
   assert(xp == startp);
   if (*xp == xarray[*xp]) {
      assert(mic_list[sacred_id].section == 2);
      sacred_edge = mic_list[sacred_id].head(mic_list);
   }
   else
      sacred_edge = NO_ID;
#endif // SYMMETRY
   sacred_span();
   if (param.verbose > 2)
      print();
   check_connections();
}

long OneAbor :: c_factor()
{
   long connection_factor = 0;
   city_a *city;

   for (city = mic_list; city < mic_list.end_city; city++)
      if (city->section != 1)
         connection_factor += ((int)city->branches-2)*((int)city->branches-2);
   return connection_factor;
}

/* sort_lambda_costs
 *
 * This function looks at the one_tree inside the node's mic_list and modifies
 * the lambdas according to Langrangean constraints. The function then sorts
 * the costs so the next call to a one_tree() or initial_one_tree() procedure
 * can use the newer better lambda costs.
 *
 * sub is the depth at which you don't assume the connections should be two.
 */
typedef struct conn_t {
#ifdef SLURR
   lambdacost_t slurr;
#else
   int brnches;
#endif
} conn_t;

static conn_t *st_branches;

int OneAbor :: sort_lambda_costs(const sum_t optimost)
{
   city_a *city;
   long connection_factor = c_factor();
   float TT;
#ifdef SLURR
   lambdacost_t slurr;
#else
   int brnches;
#endif

   if (optimost <= optimal)
      return 2;
   if (connection_factor == 0)
      return 1;

#if defined(SPECIAL0) || defined(SPECIAL3)
   TT = alpha*((float)(optimost - OPTIMAL)) / connection_factor;
#endif
#if defined(SPECIAL1) || defined(SPECIAL2)
   TT = alpha;
   alpha *= alpha_factor;
#endif
#if defined(SPECIAL1) || defined (SPECIAL0)
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section == 1)
         continue;
#ifdef SLURR
      slurr = TT*((int)city->branches - 2);
      city->lambda += (lambdacost_t) (0.7*slurr
         + st_branches[city->get_id(mic_list)].slurr);
      st_branches[city->get_id(mic_list)].slurr = .3*slurr;
#else
      brnches = (int)city->branches - 2;
      city->lambda += (lambdacost_t) (1.3*TT*brnches
         + 0.7*TT*st_branches[city->get_id(mic_list)].brnches);
      st_branches[city->get_id(mic_list)].brnches = brnches;
#endif
   }
#endif
#ifdef SPECIAL2
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section == 1)
         continue;
      brnches = ((int)city->branches - 2);
      city->lambda += (lambdacost_t) (1.2*TT*brnches
         + 0.7*TT*st_branches[city->get_id(mic_list)].brnches);
      st_branches[city->get_id(mic_list)].brnches = brnches;
   }
#endif
#ifdef SPECIAL3
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section == 1)
         continue;
      slurr = st_branches[city->get_id(mic_list)].slurr;
      slurr *= .378;
      slurr += .37*TT*((int)city->branches - 2);
      mic_list[city->get_id(mic_list)].lambda += slurr;
      st_branches[city->get_id(mic_list)].slurr = slurr;
   }
#endif

   return 0;
}

/* train lambdas with sort_lambda_cost() keeping *this as the best
 * so far, and using current to do the ascending
 *
 * Point *1*: may not have to assign this, in fact it might even be
 * bad since the last lambdas that made a tour out of the one-tree
 * were already over written, you can almost garuantee that *this
 * was assigned previously because a onetree found that is a tour is
 * always going to be higher than any minimum one-tree found that isn't
 * or was not a tour previously.
 */
int OneAbor :: train_lambdas(sum_t optimost, int max_iters)
{
   int count, iters;
   int non_improvements = 0, is_tour = 0;
   OneAbor *current_node;

   min_span();
   st_branches = new conn_t[solver->matrix->degree];
#ifdef SPECIAL0
   alpha = START_ALPHA;
   alpha_factor = ALPHA_FACTOR(START_ALPHA, END_ALPHA, 3*max_iters/4.);
#endif
#ifdef SPECIAL3
   alpha = START_ALPHA;
   alpha_factor = ALPHA_FACTOR(START_ALPHA, END_ALPHA, max_iters);
#endif
#ifdef SPECIAL1
   alpha = ((float)optimost-OPTIMAL) / (solver->matrix->degree);
   /*
   alpha_factor = .97;
    */
   alpha_factor = ALPHA_FACTOR(alpha, alpha/solver->matrix->degree,
      3*max_iters/4.);
#endif
#ifdef SPECIAL2
   double connection_factor = c_factor();
   alpha = connection_factor ? ((float)optimost-OPTIMAL)/connection_factor : 0.;
   alpha_factor = .99;
#endif
#if defined(SPECIAL2)
   /*
   double jj = 0;
   for (count = 0; count < (int)solver->matrix->degree; count++)
      jj += fabs(mic_list[count].lambda);
   jj /= solver->matrix->degree;
    */
   for (count = 0; count < (int)solver->matrix->degree; count++) {
      /*
      mic_list[count].lambda += (-jj/2.+RAND(jj));
       */
      st_branches[count].brnches = 0;
   }
#else
   for (count = 0; count < (int)solver->matrix->degree; count++) {
#ifdef SLURR
      st_branches[count].slurr = 0.;
#else
      st_branches[count].brnches = 0;
#endif
   }
#endif
   count = iters = 0;
   /*
   if (param.verbose > 1) {
    */
   if (param.verbose > 1
    && ((param.verbose > 20) ? 1 : max_iters != GENERAL_MAX_ITERATIONS)) {
      dump << "First 1-Tree = " << (int)optimal << ", Initial Step "
         << alpha << "," << alpha_factor << "\n";
      dump.flush();
   }
   current_node = new OneAbor(*this);
   do {
      if ((is_tour=current_node->sort_lambda_costs(optimost)) != 0) {
         if (is_tour != 1)
            is_tour = 0;
         else
            *this = *current_node;       /*1*/
         break;
      }
      current_node->min_span();
      if (current_node->optimal > optimal) {
         /*
         if (param.verbose > 1) {
          */
         if (param.verbose > 1
          && ((param.verbose > 20) ? 1 : max_iters != GENERAL_MAX_ITERATIONS)) {
            dump << "Tree " << iters << "::" << current_node->optimal
               << "(" << c_factor() << ") " << count
               << "\n";
         }
         if (current_node->optimal > optimal + (sum_t)OPT_DIFF_THRESHOLD)
            non_improvements = 0;
         *this = *current_node;
         count = iters;
      }
      else {
         if (non_improvements++ > OPT_DIFF_BREAKS)
            break;
#if defined(DECREASE) \
 && (defined(SPECIAL0) || defined(SPECIAL2) || defined(SPECIAL3))
/*
#ifdef SPECIAL2
         if (max_iters == GENERAL_MAX_ITERATIONS)
#endif
 */
         alpha *= alpha_factor;
         if (param.verbose > 2
          && ((param.verbose > 20) ? 1 : max_iters != GENERAL_MAX_ITERATIONS))
            dump << "Alpha " << alpha << "\n";
#endif
      }
   } while (iters++ < max_iters+1);
   if (current_node->optimal > optimal) {
      *this = *current_node;
      count = iters;
      assert(0);
   }
   min_span();
   /*
   if (param.verbose > 1) {
    */
   if (param.verbose > 1
    && ((param.verbose > 20) ? 1 : max_iters != GENERAL_MAX_ITERATIONS)) {
      dump << "FINAL LAMBDA SUM " << lambda_sum << "\n";
      dump << "ITERS " << iters << " MAX " << max_iters << " highest_opt ->["
       << optimal << "]<- Used at iters " << count << ", Final Step "
       << alpha << "\n";
      dump.flush();
   }
#if defined(SYMMETRY) && defined(HEUR)
   Tour B;
   dump << "Concrete[" << depth << ":" << optimal << "("
      << c_factor() << ")]=";
   dump.flush();
   min_matching(&B);
   dump << B.cost(solver->matrix) << "=";
   dump.flush();
   FindTour to("+ro+th+ro", solver->matrix, &solver->duration, 0, &B);
   dump << to.tour->cost(solver->matrix) << "=Jungle\n";
   if (to.tour->cost(solver->matrix) < optimost) {
      to.tour->show(solver->matrix);
      solver->update_tour(to.tour, 1);
   }
#endif
   delete current_node;
   delete st_branches;
   check_connections();
   return is_tour;
}

inline void OneAbor :: init_first_constructor(city_id_t, TSPSolver *s)
{
   solver = (OneAborSolver*)s;
   install_blocked_mallocs(solver->matrix->degree*sizeof(city_a), 100);
   mic_list.init(solver->matrix->degree);
   depth = 0;
   required_cost = 0;
   sacred_id = NO_ID;
}

OneAbor :: OneAbor (city_id_t init_id, TSPSolver *s, sum_t &opt)
{
   init_first_constructor(init_id, s);
#ifdef NODOTRAIN
   min_span();
#else
/*
min_span();
Term file(0);
file.open("FILE", "w", 0);
plot_tree(file);
file.close();
 */
   if (train_lambdas(opt, INITIAL_MAX_ITERATIONS))
      depth = (city_id_t)(solver->matrix->degree-2);
   else
#endif
   /*
   if (param.most == MAX_SUM)
    */
   if (solver->matrix->degree > 15)
      heuristic_tour(solver->tour, opt);

   order(optimal, depth);
}

OneAbor :: OneAbor (TSPSolver *s, Tour *tour)
{
   init_first_constructor(0, s);
   min_span();
   order(optimal, depth);
   min_matching(tour);
}

OneAbor :: OneAbor (city_id_t init_id, TSPSolver *s)
{
   init_first_constructor(init_id, s);
   min_span();
   order(optimal, depth);
}

OneAbor :: OneAbor (TSPSolver *s)
{
   init_first_constructor(NO_ID, s);
   optimal = 0;
   order(optimal, depth);
}

/* this procedure is called after all cuts are made from the onetree,
 * and after this is called a new minimum spaning tree will be mended.
 *
 * We always assume the micmap has been created before inc_min_span is called.
 * The cuts will assumed to have been excluded edges or hidden edges
 * from requiring an edge.  the a_root passed needs to be a root for
 * one of the trees that needs to be mended.
 */
inline void OneAbor :: incremental_min_span()
{
}

void OneAbor :: construct(const OneAbor &parent)
{
   solver = parent.solver;
   mic_list.init(solver->matrix->degree);
   *this = parent;
}

// requireing an edge as a sort of simple operation with bad
// cases.  Overall, you degrade sections of both cities, and then
// if either of the cities is section3, then you must forbid all edges
// from that city.  ASSUMING: edge a,b is already in the one-tree
//
// Point *1*: if both are section2 then you are connection two partial tours.
// first make the other ends of a and b have ends of the other.  Then,
// cut all the edges off that go to each of them
//
// Point *2*: if one is greater than the other we know they are 2 and 3.
// we make cityb be the section3 and citya be the section2.  citya's end
// will change its end id to b.  We cut all edges from citya. and then degrade.
//
void OneAbor :: require_edge(city_id_t a, city_id_t b)
{
   city_a *citya = mic_list+a, *cityb = mic_list+b;
   city_id_t up;

   required_cost += solver->matrix->val(a, b);
   if (citya->is_section2() && cityb->is_section2()) { /*1*/
#ifdef SYMMETRY
      mic_list[cityb->u.sec2.end].section = citya->section;
#else
      assert(citya->section == 4 && cityb->section == 2);
      assert(mic_list[cityb->u.sec2.end].section == citya->section);
#endif
      assert(cityb->u.sec2.end != citya->get_id(mic_list));
      assert(citya->u.sec2.end != cityb->get_id(mic_list));
      assert(mic_list[citya->u.sec2.end].is_section2() &&
         mic_list[citya->u.sec2.end].u.sec2.end == citya->get_id(mic_list));
      assert(mic_list[cityb->u.sec2.end].is_section2() &&
         mic_list[cityb->u.sec2.end].u.sec2.end == cityb->get_id(mic_list));
      mic_list[citya->u.sec2.end].u.sec2.end = cityb->u.sec2.end;
      mic_list[cityb->u.sec2.end].u.sec2.end = citya->u.sec2.end;
      citya->branches = cityb->branches = 2;
      citya->section = cityb->section = 1;
      up = citya->u.sec2.required_edge;
      citya->u.sec1.required_edge1 = up;
      citya->u.sec1.required_edge2 = b;
      up = cityb->u.sec2.required_edge;
      cityb->u.sec1.required_edge2 = up;
      cityb->u.sec1.required_edge1 = a;
   }
   else if (citya->section == cityb->section) {
      assert(citya->section == 3 && cityb->section == 3);
      citya->section = 2;
      citya->u.sec2.required_edge = citya->u.sec2.end = b;
      cityb->section = 4;
      cityb->u.sec2.required_edge = cityb->u.sec2.end = a;
   }
   else if (citya->section == 3) {
#ifdef SYMMETRY
      citya->section = cityb->section;
      assert(cityb->is_section2());
#else
      assert(cityb->section == 2);
      citya->section = 2;
#endif
      assert(mic_list[cityb->u.sec2.end].is_section2() &&
         mic_list[cityb->u.sec2.end].u.sec2.end == cityb->get_id(mic_list));
      mic_list[citya->u.sec2.end = cityb->u.sec2.end].u.sec2.end = a;
      cityb->branches = 2;
      up = cityb->u.sec2.required_edge;
      cityb->u.sec1.required_edge2 = up;
      cityb->u.sec1.required_edge1 = a;
      cityb->section = 1;
      citya->u.sec2.required_edge = b;
   }
   else {
#ifdef SYMMETRY
      assert(citya->is_section2() && cityb->section == 3);
      cityb->section = citya->section;
#else
      assert(citya->section == 4 && cityb->section == 3);
      cityb->section = 4;
#endif
      assert(mic_list[citya->u.sec2.end].is_section2() &&
         mic_list[citya->u.sec2.end].u.sec2.end == citya->get_id(mic_list));
      mic_list[cityb->u.sec2.end = citya->u.sec2.end].u.sec2.end = b;
      citya->branches = 2;
      up = citya->u.sec2.required_edge;
      citya->u.sec1.required_edge1 = up;
      citya->u.sec1.required_edge2 = b;
      citya->section = 1;
      cityb->u.sec2.required_edge = a;
   }
}

void OneAbor :: forbid_edge(const city_id_t a, const city_id_t b)
{
#ifdef PRINT_IT
if (param.verbose > 1)
   dump << "Forbidding (" << a << "," << b << "\n";
#endif
#ifdef SYMMETRY
   mic_list[a].fs.forbid(b);
#endif
   mic_list[b].fs.forbid(a);
}

OneAbor :: OneAbor (const OneAbor &parent, const city_id_t a, const city_id_t b,
   const city_id_t c, const city_id_t d, int parm)
{
   construct(parent);
   if (parm) {
#ifdef PRINT_IT
if (param.verbose > 1)
   dump << "Requiring (" << a << "," << b << "),(" << c << "," << d << ")\n";
#endif
      require_edge(a, b);
      require_edge(c, d);
      depth += (city_id_t)2;
      min_span();
      trip("RR  ");
      order(max(optimal, ord.sum), depth);
   }
   else {
      rebirth(a, b, c, d);
   }
}

OneAbor :: OneAbor (const OneAbor &parent, const city_id_t a, const city_id_t b,
   int parm)
{
   construct(parent);
   if (parm) {
      forbid_edge(a, b);
      min_span();
      trip(" F  ");
      order(max(optimal, ord.sum), depth);
   }
   else {
      rebirth(a, b);
   }
}

/* Require the first and forbid the second
 *
 * Point *1*: the require_edge() should happen after the forbid edge since
 * the if require happens first it might move around the miclist under the
 * b the pivot point would be dangling.  Note: this does not happen when
 * we don't use a pivot pointer and just use the id 'b' like we are doing now.
 */
OneAbor * OneAbor :: rebirth(const city_id_t a, const city_id_t b,
   const city_id_t c, const city_id_t d)
{
   forbid_edge(c, d); /*1*/
#ifdef PRINT_IT
if (param.verbose > 1)
   dump << "Requiring (" << a << "," << b << ")\n";
#endif
   require_edge(a, b);
   depth++;
   min_span();
   trip("RF  ");
   order(max(optimal, ord.sum), depth);
   return this;
}

OneAbor * OneAbor :: rebirth(const city_id_t a, const city_id_t b)
{
   /*
   city_id_t up_a, up_b;
   int respan = (mic_list[a].is_section2() && mic_list[a].branches > 2)
    || (mic_list[b].is_section2() && mic_list[b].branches > 2);

   if (!respan) {
      up_a = mic_list[a].up();
      up_b = mic_list[b].up();
   }
    */
#ifdef PRINT_IT
if (param.verbose > 1)
   dump << "Requiring (" << a << "," << b << ")\n";
#endif
   require_edge(a, b);
   depth++;
   /*
   if (respan) {
    */
      min_span();
   /* Can't do this fancy stuff because you can required two edges that
    * can be poiting to eachother in the onetree, and that edge must be
    * forbidden, and we don't know how to quickly do it here.
   }
   else {
      if (mic_list[a].is_section2()) {
         if (up_a == b)
            mic_list[a].u.sec2.onetree_edge = NO_ID;
         else {
            mic_list[a].u.sec2.onetree_edge = up_a;
         }
      }
      if (mic_list[b].is_section2()) {
         if (up_b == a)
            mic_list[b].u.sec2.onetree_edge = NO_ID;
         else
            mic_list[b].u.sec2.onetree_edge = up_b;
      }
      check_connections();
   }
    */
   trip(" R  ");
   order(max(optimal, ord.sum), depth);
   return this;
}

OneAbor :: OneAbor(const OneAbor &parent)
{
   construct(parent);
}

OneAbor :: OneAbor(const OneAbor &parent, int)
{
   solver = parent.solver;
   mic_list.init(solver->matrix->degree);
   *this = parent;
}

oneabor_stats st_astats;

void init_oneabor_stats()
{
   short x;
   st_astats.Trains = st_astats.Explores = 0;
   for (x = 0; x<MAX_OT_STATS; x++) {
      st_astats.dist[x] = 0;
   }
}

void free_oneabor_stats(Term &term)
{
   short x;
   term << "Ostats : ";
   term << "Trained " << ((st_astats.Explores == 0) ? (double)0.0 :
      (double)st_astats.Trains*(double)100.0/(double)st_astats.Explores)
      << "% of " << (double)st_astats.Explores << " Explores\n";
   for (x = 0; x<MAX_OT_STATS; x++) {
      if (st_astats.dist[x] != 0) {
         term << x << "%==" << st_astats.dist[x] << ", ";
      }
   }
   term << "\n";
}

// Try to train this subproblem
#ifdef NODOTRAIN
int OneAbor :: try_train(const sum_t)
{
#else
int OneAbor :: try_train(const sum_t most)
{
   if (stat_will_train(most)) {
      st_astats.Trains++;
      if (param.verbose > 2)
         dump << "TRAIN";
      if (train_lambdas(most, GENERAL_MAX_ITERATIONS))
         return 1;
   }
#endif
   return c_factor() == 0; // Fix: c_factor() doesn't have to be called
                           // here because who calls it can already know
}

/* Point *1*: find a section3 or preferably a section2 city and mark the downs.
 *
 * Here we also Never choose the sacred edge as being the chosen
 * edges for exclusion and inclusion since it makes it too complicated
 * to include a sacred edge.
 *
 * Point *2*: makes certain of this point.  If we find the city we choose
 * has only three branches and one of them is a section2 branch, this is
 * a case where we will have to take the sacred edge and branch with it.
 * We can however branch such that we do not have to require the sacred
 * edge but only exclude it.
 *
 * Point *3*: if savecity is root, thus it has sacred edge pointing to it.
 *
 * Point *6*: if can't require first edge (is section to going away from edge
 * to require), then flip flop asserting second is requirable and disable
 * double require.
 *
 * Point *7*: If still section two then don't Require both edges if secondb is
 * other end of section two.
 */
void OneAbor :: branch(PQueue &que)
{
   city_a *city;
   city_id_t x, firsta, firstb;
   OneAbor *new_node;
   city_id_t seconda, secondb;
   city_a *savecity = NULL;
   sum_t prevoptimal = optimal;
   city_id_t *down = solver->line;
#ifdef SYMMETRY
   city_id_t *down2 = solver->shadow;
#endif

#ifdef PRINT_IT
if (param.verbose)
print();
#endif
   if (try_train(que.most)) {
      st_astats.Explores++;
      depth = (city_id_t)(solver->matrix->degree-1);
      order(max(optimal, ord.sum), depth);
      que.enq(this);
      que.dont_delete_last_dequeued();
      return;
   }
   if (prevoptimal > optimal) {
      dump << "DIE\n";
   }
   /*
   if (fequiv(ord.sum, 2099.07302, .001)
    || fequiv(optimal, 2099.07302, .001)) {
      solver->duration.stop();
      dump << optimal << " ~ " << ord.sum <<" Again?("
       << c_factor() << ")[" << depth << "]\n";
      Term file(0);
      file.open("FILE", "w", 0);
      plot_tree(file);
      file.close();
      solver->duration.start();
   }
    */
#ifdef PRINT_IT
if (param.verbose)
print();
#endif
   trip("BRC ");
#if defined(USE_BRANCH_SECORD)
   compute_secords();
#endif
   for (x = 0; x < solver->matrix->degree; x++) {
      down[x] = NO_ID;
   }
   for (city = mic_list; city < mic_list.end_city; city++) { /*1*/
# ifdef SYMMETRY
      if (city->section != 1 && (x = city->up()) != NO_ID) {
         down2[x] = down[x];
         down[x] = city->get_id(mic_list);
      }
# else
      if (city->section != 1 && (x = city->up_root(mic_list)) != NO_ID
       && city->section != 4)
         down[x] = city->get_id(mic_list);
# endif
   }
   for (city = mic_list; city < mic_list.end_city; city++) { /*1*/
      if (city->branches > 2) {
#  ifdef SYMMETRY
         assert(city->get_id(mic_list) != sacred_id);
#  endif
         savecity = city;
         /*
         x = ((city->up() != NO_ID) ? 1 : 0)
          + ((down[city->get_id(mic_list)] != NO_ID)
          ? ((down2[city->get_id(mic_list)] != NO_ID) ? 2 : 1)
          : 0) + city->is_section2() ? 4 : 0;
         if (x > y || savecity == NULL) {
            y = x;
            savecity = city;
         }
          */
      }
   }
   if ((city = savecity) == NULL) {
      st_astats.Explores++;
      depth = (city_id_t)(solver->matrix->degree-1);
      order(max(optimal, ord.sum), depth);
      que.enq(this);
      que.dont_delete_last_dequeued();
      return;
   }
# ifdef SYMMETRY
   firsta = seconda = city->get_id(mic_list);
   assert(seconda != sacred_id);
   /*
   if (seconda == sacred_edge) {
      firstb = sacred_id;
      if ((secondb = city->up()) == NO_ID)
         secondb = down[firsta];
   }
   else {
      if ((firstb = city->up()) == NO_ID) {
         firstb = down[firsta];
         secondb = down2[firsta];
      }
      else {
         secondb = down[firsta];
      }
      assert(firstb != NO_ID);
   }
   assert(secondb != NO_ID);
    */
   if ((firstb = city->up()) == NO_ID) {
      firstb = down[firsta];
      assert(firstb != NO_ID);
      if (seconda == sacred_edge)
         secondb = sacred_id;
      else {
         secondb = down2[firsta];
         assert(secondb != NO_ID);
      }
   }
   else if ((secondb = down[firsta]) == NO_ID) {
      assert(seconda == sacred_edge);
      secondb = sacred_id;
   }
   if (firstb != secondb && (mic_list[secondb].is_section2()
    ? mic_list[secondb].u.sec2.end != firstb : 1) 
    && (mic_list[seconda].is_section2() ? seconda != firsta : 1)) {
   /*
   if (city->section == 3 && (mic_list[secondb].is_section2()
    ? (mic_list[secondb].u.sec2.end != firstb) : 1) && firstb != secondb) {
     */
# else // SYMMETRY
   firsta = city->tail(mic_list);
   seconda = city->head(mic_list);
   if (down[seconda] == NO_ID)
      seconda = firsta;
   if ((x = city->up_root(mic_list)) == NO_ID) { /*3*/
      assert(down[seconda] != NO_ID);
      secondb = down[seconda];
      assert(firsta == sacred_id);
      firstb = sacred_edge;
      if (firstb == seconda) {
         x = firstb;
         firstb = seconda;
         seconda = x;
         x = firsta;
         firsta = secondb;
         secondb = x;
      }
   }
   else if (down[seconda] == NO_ID) {
      /*
      secondb = mic_list[x].head(mic_list);
       */
      secondb = x;
      firstb = sacred_edge;
   }
   else {
      /*
      firstb = mic_list[x].head(mic_list);
       */
      firstb = x;
      secondb = down[seconda];
   }
   if (mic_list[firstb].section == 2) { /*6*/
      x = firstb;
      firstb = seconda;
      seconda = x;
      x = firsta;
      firsta = secondb;
      secondb = x;
      assert(mic_list[firstb].section != 2);
      x = 0;
   }
   else
      x = (mic_list[firstb].section == 4
       && mic_list[firstb].u.sec2.end == secondb) ? 0 : NO_ID; /*7*/

   // branch by requiring two edges
   if (firstb != secondb && ((city->section == 3) ? 1 : (firsta != seconda))
    && x == NO_ID) {
    /*
    && !(mic_list[firstb].is_section2()
    && mic_list[firstb].u.sec2.end == secondb)) {
     */
# endif // SYMMETRY
      st_astats.Explores++;
      new_node = new OneAbor(*this, firstb, firsta, seconda, secondb, 1);
      if (que.enq(new_node) && new_node->optimal < prevoptimal && param.verbose)
         dump << " - RR PREV " << prevoptimal << " > ["
            << new_node->c_factor() << "] " << new_node->optimal << "\n";
   }

   // branch by forbiding second
   st_astats.Explores++;
   new_node = new OneAbor(*this, firstb, firsta, 1);
   if (que.enq(new_node) && new_node->optimal < prevoptimal && param.verbose)
      dump << " -  F PREV " << prevoptimal << " > ["
         << new_node->c_factor() << "] " << new_node->optimal << "\n";

   // branch by requireing second and forbiding first
   st_astats.Explores++;
   new_node = rebirth(firstb, firsta, seconda, secondb);
   if (que.enq(new_node) && new_node->optimal < prevoptimal && param.verbose)
      dump << " - RF PREV " << prevoptimal << " > ["
         << new_node->c_factor() << "] " << new_node->optimal << "\n";
   que.dont_delete_last_dequeued();
}

sum_t OneAbor :: sym_branch_measure(
   const city_id_t firstb, const city_id_t firsta,
   const city_id_t seconda, const city_id_t secondb,
   OneAbor *&node_one, OneAbor *&node_two, OneAbor *&node_three)
{
   OneAbor *n1 = new OneAbor(*this, secondb, seconda, 1);
   OneAbor *n2 = new OneAbor(*this, secondb, seconda, firsta, firstb, 0);
   sum_t one, two, three, ret, n_one = n1->optimal, n_two = n2->optimal;

   // branch by forbiding first
   node_one = new OneAbor(*this, firstb, firsta, 1);
   one = node_one->optimal;
   // branch by requireing first and forbiding second
   node_two = new OneAbor(*this, firstb, firsta, seconda, secondb, 0);
   two = node_two->optimal;
   if (one < two) {
      ret = one;
      one = two;
      two = ret;
   }
   if (n_one < n_two) {
      ret = n_one;
      n_one = n_two;
      n_two = ret;
   }
   if (n_one > one && n_two >= two) {
      one = n_one;
      two = n_two;
      delete node_two;
      delete node_one;
      node_one = n1;
      node_two = n2;
   }
   else {
      delete n1;
      delete n2;
   }
   if (firstb != secondb && (!mic_list[secondb].is_section2()
    || mic_list[secondb].u.sec2.end != firstb) 
    && (!mic_list[seconda].is_section2() || seconda != firsta)) {
      node_three = new OneAbor(*this, firstb, firsta, seconda, secondb, 1);
      three = node_three->optimal;
      if (three > one) {
         ret = two;
         two = one;
         one = three;
         three = ret;
      }
      else if (three > two) {
         ret = two;
         two = three;
         three = ret;
      }
      ret = (sum_t)((one + 2*two + 3.*three)/7.);
   }
   else {
      ret = (sum_t)((one + 2.*two)/3.);
   }
   return ret;
}

#define MINLSUM MIN_SUM
/*
#define MINLSUM MINLCOST
 */

#define MAX_BRANCHES 7

// Finds all branches, then finds combinations of pairs of branches from all
// possible, and finds best branch_measure from each pair.
//
void OneAbor :: sym_best_branch(city_a *city, sum_t &best,
   OneAbor *&node1, OneAbor *&node2, OneAbor *&node3)
{
   OneAbor *n1, *n2, *n3;
   city_id_t *down = solver->line, *down2 = solver->shadow;
   city_id_t fa, b1[MAX_BRANCHES*2], *bs = b1, *bs2, *be;
   city_id_t id = city->get_id(mic_list);
   sum_t l;

   if ((fa = city->up()) != NO_ID) {
      *bs++ = id;
      *bs++ = fa;
   }
   if ((fa = down[id]) != NO_ID) {
      *bs++ = id;
      *bs++ = fa;
      if ((fa = down2[id]) != NO_ID) {
         *bs++ = id;
         *bs++ = fa;
      }
   }
   if (id == sacred_edge) {
      *bs++ = id;
      *bs++ = sacred_id;
   }
   if (city->is_section2()) {
      id = city->u.sec2.end;
      city = mic_list+id;
      if ((fa = city->up()) != NO_ID) {
         *bs++ = id;
         *bs++ = fa;
      }
      if ((fa = down[id]) != NO_ID) {
         *bs++ = id;
         *bs++ = fa;
         if ((fa = down2[id]) != NO_ID) {
            *bs++ = id;
            *bs++ = fa;
         }
      }
      if (id == sacred_edge) {
         *bs++ = id;
         *bs++ = sacred_id;
      }
   }
   assert(bs-b1 <= MAX_BRANCHES*2 && bs-b1 > 2);
   n1 = n3 = NULL;
   for (be = bs, bs = b1; bs < be; bs += 2) {
      for (bs2 = bs + 2; bs2 < be; bs2 += 2) {
         if (best < (l
          = sym_branch_measure(bs[1], *bs, *bs2, bs2[1], n1, n2, n3))) {
            if (best != MINLSUM) {
               delete node1; delete node2;
               if (node3 != NULL)
                  delete node3;
            }
            best = l; node1 = n1; node2 = n2; node3 = n3;
            n1 = n3 = NULL;
         }
         else if (n1 != NULL) {
            delete n1; delete n2;
            if (n3 != NULL) {
               delete n3;
               n3 = NULL;
            }
         }
#ifdef PRINT_IT
if (param.verbose>1)
dump << "------" << l << "------\n";
#endif
      }
   }
}

void OneAbor :: sym_branch(PQueue &que)
{
   city_a *city;
   city_id_t x;
   OneAbor *node1, *node2, *node3;
   sum_t best = MINLSUM;
   sum_t prevoptimal = optimal;
   city_id_t *down = solver->line, *down2 = solver->shadow;

   node1 = node2 = node3 = NULL;
   if (try_train(que.most)) {
      st_astats.Explores++;
      depth = (city_id_t)(solver->matrix->degree-1);
      order(max(optimal, ord.sum), depth);
      que.enq(this);
      que.dont_delete_last_dequeued();
      return;
   }
   if (prevoptimal > optimal) {
      dump << "DIE\n";
   }
   trip("BRC ");
   for (x = 0; x < solver->matrix->degree; x++) {
      down[x] = NO_ID;
   }
   for (city = mic_list; city < mic_list.end_city; city++) { /*1*/
      if (city->section != 1 && (x = city->up()) != NO_ID) {
         down2[x] = down[x];
         down[x] = city->get_id(mic_list);
      }
   }
   for (city = mic_list; city < mic_list.end_city; city++) { /*1*/
      if (city->section == 1 || city->section == 2)
         continue;
      if (city->branches > 2 || (city->section == 4
       && mic_list[city->u.sec2.end].branches > 2)) {
         sym_best_branch(city, best, node1, node2, node3);
      }
   }
#ifdef PRINT_IT
if (param.verbose>1)
dump << "++++++" << best << "++++++\n";
#endif
   assert(best != MINLSUM);
   if (node3 != NULL) {
      st_astats.Explores++;
      if (que.enq(node3) && node3->optimal < prevoptimal && param.verbose)
         dump << " - RR PREV " << prevoptimal << " > ["
            << node3->c_factor() << "] " << node3->optimal << "\n";
   }
   st_astats.Explores++;
   if (que.enq(node1) && node1->optimal < prevoptimal && param.verbose)
      dump << " -  F PREV " << prevoptimal << " > ["
         << node1->c_factor() << "] " << node1->optimal << "\n";
   st_astats.Explores++;
   if (que.enq(node2) && node2->optimal < prevoptimal && param.verbose)
      dump << " - RF PREV " << prevoptimal << " > ["
         << node2->c_factor() << "] " << node2->optimal << "\n";
//   que.dont_delete_last_dequeued();
}

void OneAbor :: write(BinFile &t) const
{
   t << optimal << depth
      << required_cost << sacred_id << sacred_edge << lambda_sum;
#ifdef SECORD
   t << sacred_secord;
#endif
   mic_list.write(t);
}

PQuelem *OneAbor :: read_clone(BinFile &t, TSPSolver *s) const
{
   city_a *city;
   OneAbor *o = new OneAbor;

   t >> o->optimal >> o->depth
      >> o->required_cost >> o->sacred_id >> o->sacred_edge >> o->lambda_sum;
#ifdef SECORD
   t >> o->sacred_secord;
#endif
   o->solver = (OneAborSolver*)s;
   o->mic_list.init(o->solver->matrix->degree);
   for (city = o->mic_list; city < o->mic_list.end_city; city++)
      city->read(t);
   return o;
}

OneAbor :: OneAbor (const OneAbor &parent, const city_id_t sub,
   const city_id_t k, Tour *t)
{
   city_id_t count, beg_city, sec_city;
   Path *p, *n, *c;
   city_a *ml;

   construct(parent);

   c = t->get_head();
   p = c->get_prev();
   for (count = 0, n = c->get_next(); count < sub; count++) {
      p = c;
      n = (c = n)->get_next();
   }
   required_cost = solver->matrix->val(p->id, c->id);
   beg_city = p->id;
   sec_city = c->id;
   for (count = solver->matrix->degree; --count > k+1; ) {
      required_cost += solver->matrix->val(c->id, n->id);
      ml = mic_list+c->id;
      ml->u.sec1.required_edge1 = p->id;
      ml->u.sec1.required_edge2 = n->id;
      ml->branches = 2;
      ml->section = 1;
      p = c;
      n = (c = n)->get_next();
   }
   ml = mic_list+c->id;
   ml->u.sec2.required_edge = p->id;
   ml->u.sec2.end = beg_city;
   mic_list[beg_city].u.sec2.required_edge = sec_city;
   mic_list[beg_city].u.sec2.end = c->id;
   ml->branches = ml[1].branches = 1;
   ml->u.sec2.onetree_edge = ml[1].u.sec2.onetree_edge = NO_ID;
   ml->section = ml[1].section = 2;
   depth = solver->matrix->degree - k - 1;
// min_span();
   train_lambdas(t->cost(solver->matrix), GENERAL_MAX_ITERATIONS);
   order(optimal, depth);
}

/* Christofides Algorithm
 *
 * This finds a minimum matching between pairs of cities that have verticies
 * of odd degree of the current spanning tree and then adds the matching to the
 * It then builds a tour from the eulerian tour made by the matching and tree.
 *
 * The algorithm starts by randomly grouping the odd cities in to pairs.
 * It then starts to make a minimum matching between two pairs, which can
 * be done two other ways than how they are matched already, i.e. (AB, CD)
 * (AC, BD) (AD, BC).  We now have a matching that is minimum and we procede
 * by combining an un-matched pair into the minimized set building a larger
 * minimized matching untill all pairs are minimized.
 *
 * If there is a minimized matching of N pairs and there needs to be introduced
 * a new pair into the matching, there are 4*N*(N-1)/2 + 2*N other ways
 * to combine new pair into the current matching.   The 2*N ways are from
 * going through each of the pairs and minimize that pair with the new pair,
 * as above.  The 8*N*(N-1)/2 ways are 8 other ways of introducing the new
 * pair into two distinct pairs already in the matching, i.e. (AB, CD, EF)
 * (AC, BE, DF) (AC, DE, BF) (AD, BE, CF) (AD, CE, BF)
 * (AE, BC, FD) (AE, FC, BD) (AF, BC, ED) (AF, EC, BD)
 *
 * Once we have add the matching to the spanning tree, we can follow all edges
 * marking them as used and create a tour by marking all the cities we haven't
 * traveled yet.  If we follow edges that haven't been used yet untill all
 * edges are used, we will vistit all the cities .
 *
 * Point *1*: assign the first matching
 * Point *2*: Choose first already matched
 */
void OneAbor :: min_matching(Tour *ret)
{
   city_id_t cons = 0, here[4];
   AMatching *matchings, *con1, *con2, *con3, *conend, *best_con2;
   AMatching *best_con3 = NULL;
   sum_t sofar, sofar_best, tmp, almost, almost_b2;
   cost_t *con1b2, *con1b1, *con2b2, *con2b1;
   Tour traveled(solver->matrix->degree);
   city_a *city;
   const Matrix *matrix = solver->matrix;

#ifdef NO_ARRAY_CONSTRUCTING
   assert(0);
#endif
   matchings = new AMatching[solver->matrix->degree];
   for (city = mic_list; city < mic_list.end_city; city++) {
      if (city->section != 1 && (city->branches - ((city->get_id(mic_list)
       == sacred_id || city->get_id(mic_list) == sacred_edge) ? 1 : 0)) % 2) {
         if (!(cons % 2))
            matchings[cons/2].b1 = city->get_id(mic_list);
         else {
            con1 = matchings+(cons/2);
            con1->b2 = city->get_id(mic_list);
            con1->u.concost = matrix->val(con1->b1, con1->b2);
         }
         cons++;
      }
   }
   assert(!(cons % 2));
   conend = matchings+(cons/2);
   con1 = matchings;
   sofar_best = con1->u.concost;
   for (con1++; con1 < conend; con1++) {
      sofar = sofar_best;
      sofar_best = sofar + con1->u.concost; // (AB, CD, EF)
      best_con2 = NULL;
      con1b2 = matrix->cost[con1->b2];
      con1b1 = matrix->cost[con1->b1];
      for (con2 = matchings; con2 < con1; con2++) {
         almost = sofar - con2->u.concost;
         con2b2 = matrix->cost[con2->b2];
         con2b1 = matrix->cost[con2->b1];
         if (sofar_best > (tmp = almost + con1b1[con2->b1]
          + con1b2[con2->b2])) { // (AC, BD, EF)
            sofar_best = tmp; best_con2 = con2; best_con3 = NULL;
            here[0] = con2->b1; here[1] = con2->b2;
         }
         if (sofar_best > (tmp = almost + con1b1[con2->b2]
          + con1b2[con2->b1])) { // (AD, BC, EF)
            sofar_best = tmp; best_con2 = con2; best_con3 = NULL;
            here[0] = con2->b2; here[1] = con2->b1;
         }
         for (con3 = con2+1; con3 < con1; con3++) {
            almost_b2 = almost - con3->u.concost;
            if (sofar_best > (tmp = almost_b2 + con2b2[con1->b1]
             + con2b1[con3->b1] + con1b2[con3->b2])) { // (AD, BF, CE)
               sofar_best = tmp; best_con2 = con2; best_con3 = con3;
               here[0] = con2->b2; here[1] = con3->b2;
               here[2] = con2->b1; here[3] = con3->b1;
            }
            if (sofar_best > (tmp = almost_b2 + con2b2[con1->b1]
             + con2b1[con3->b2] + con1b2[con3->b1])) { // (AD, BE, CF)
               sofar_best = tmp; best_con2 = con2; best_con3 = con3;
               here[0] = con2->b2; here[1] = con3->b1;
               here[2] = con2->b1; here[3] = con3->b2;
            }
            if (sofar_best > (tmp = almost_b2 + con2b1[con1->b1]
             + con2b2[con3->b1] + con1b2[con3->b2])) { // (AC, BF, DE)
               sofar_best = tmp; best_con2 = con2; best_con3 = con3;
               here[0] = con2->b1; here[1] = con3->b2;
               here[2] = con2->b2; here[3] = con3->b1;
            }
            if (sofar_best > (tmp = almost_b2 + con2b1[con1->b1]
             + con2b2[con3->b2] + con1b2[con3->b1])) { // (AC, BE, DF)
               sofar_best = tmp; best_con2 = con2; best_con3 = con3;
               here[0] = con2->b1; here[1] = con3->b1;
               here[2] = con2->b2; here[3] = con3->b2;
            }
            if (sofar_best > (tmp = almost_b2 + con2b2[con1->b1]
             + con2b1[con3->b1] + con1b2[con3->b2])) { // (AF, BD, EC)
               sofar_best = tmp; best_con2 = con2; best_con3 = con3;
               here[0] = con3->b2; here[1] = con3->b2;
               here[2] = con3->b1; here[3] = con3->b1;
            }
            if (sofar_best > (tmp = almost_b2 + con2b2[con1->b1]
             + con2b1[con3->b2] + con1b2[con3->b1])) { // (AF, BC, ED)
               sofar_best = tmp; best_con2 = con2; best_con3 = con3;
               here[0] = con3->b2; here[1] = con2->b1;
               here[2] = con3->b1; here[3] = con2->b2;
            }
            if (sofar_best > (tmp = almost_b2 + con2b1[con1->b1]
             + con2b2[con3->b1] + con1b2[con3->b2])) { // (AE, BD, FC)
               sofar_best = tmp; best_con2 = con2; best_con3 = con3;
               here[0] = con3->b1; here[1] = con2->b2;
               here[2] = con3->b2; here[3] = con2->b1;
            }
            if (sofar_best > (tmp = almost_b2 + con2b1[con1->b1]
             + con2b2[con3->b2] + con1b2[con3->b1])) { // (AE, BC, FD)
               sofar_best = tmp; best_con2 = con2; best_con3 = con3;
               here[0] = con3->b1; here[1] = con2->b1;
               here[2] = con3->b2; here[3] = con2->b2;
            }
         }
      }
      if (best_con2 != NULL) {
#ifdef MINPRINT
cost_t save = con1->u.concost;
tmp = con1->u.concost + best_con2->u.concost
+ (best_con3 == NULL ? 0 : best_con3->u.concost);
#endif
         best_con2->b1 = con1->b2;
         con1->b2 = here[0]; best_con2->b2 = here[1];
         if (best_con3 != NULL) {
            best_con3->b1 = here[2];
            best_con3->b2 = here[3];
            best_con3->u.concost = matrix->val(best_con3->b1, best_con3->b2);
         }
         con1->u.concost = matrix->val(con1->b1, con1->b2);
         best_con2->u.concost = matrix->val(best_con2->b1, best_con2->b2);
#ifdef MINPRINT
tmp -= con1->u.concost + best_con2->u.concost
+ (best_con3 == NULL ? 0 : best_con3->u.concost);
dump << tmp << " == " << (sofar + save - sofar_best) << "\n";
#endif
      }
   }
   for (con1 = matchings; con1 < conend; con1++) {
      matchings[con1->b1].match = con1->b2;
      matchings[con1->b2].match = con1->b1;
   }
   for (con1 = matchings, city = mic_list; city < mic_list.end_city; city++) {
      city->place(matchings, con1, mic_list);
   }
#ifdef MINPRINT
for (con1 = matchings; con1 < matchings+solver->matrix->degree; con1++) {
   con1->print(matchings);
}
   matchings->depth_first(matchings, &traveled, NULL, 1);
#else
   matchings->depth_first(matchings, &traveled, NULL, 0);
#endif
   if (ret != NULL)
      ret->copy(traveled);
   delete matchings;
}

#define DIST 5.
void OneAbor :: plot_object(pos_t &P, pos_t *Pp, int square, Term &file)
{
   pos_t p;
   if (Pp != NULL) {
      pos_t p2;
      p2.x = (P.x + Pp->x)/2.; p2.y = (P.y + Pp->y)/2.;
      p.x = p2.x; p.y = p2.y + DIST/10.; file << p;
      file << *Pp;
      p.x = p2.x; p.y = p2.y - DIST/10.; file << p;
   }
   else {
      p.x = P.x; p.y = P.y + DIST; file << p;
      if (square) {
         p.x = P.x + DIST; p.y = P.y + DIST; file << p;
         p.x = P.x + DIST; p.y = P.y - DIST; file << p;
         p.x = P.x - DIST; p.y = P.y - DIST; file << p;
         p.x = P.x - DIST; p.y = P.y + DIST; file << p;
      }
      else {
         p.x = P.x + DIST; p.y = P.y; file << p;
         p.x = P.x; p.y = P.y - DIST; file << p;
         p.x = P.x - DIST; p.y = P.y; file << p;
      }
      p.x = P.x; p.y = P.y + DIST; file << p;
   }
   file << P;
}

void OneAbor :: plot_tree(Term &file)
{
   const city_a *city, *site;
   city_id_t *ids = new city_id_t[solver->matrix->degree + 1], *id, i;

   for (city = mic_list; city < mic_list.end_city; city++) {
      if (((city->get_id(mic_list) == sacred_id
       || city->get_id(mic_list) == sacred_edge)
       ? city->branches - 1 : city->branches) == 1) {
         *(id = ids) = city->get_id(mic_list);
         i = city->going_up(*id);
         dump << (*id) << "+";;
         for (id++; i != NO_ID; id++) {
            site = mic_list + i;
            *id = i;
            dump << i << "+";;
            i = site->going_up(id[-1]);
            assert(id - ids < solver->matrix->degree + 1);
         }
         dump << "\n";
         *id = NO_ID;
         plot_object(solver->matrix->pos[id[-1]], NULL, 1, file);
         do {
            id--;
            file << solver->matrix->pos[*id];
         } while (id > ids);
         plot_object(solver->matrix->pos[*id], NULL, 0, file);
         if (*id == sacred_id) {
            plot_object(solver->matrix->pos[sacred_id],
             solver->matrix->pos+sacred_edge, 0, file);
         }
         for (; *id != NO_ID; id++) {
            file << solver->matrix->pos[*id];
         }
      }
   }
   delete ids;
}
