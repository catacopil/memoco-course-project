/* onetreeq.cc
 *
 * IMPROVED the 1-TREE que element
 *
 * orig ceh
 */
#include <math.h>
#include "findtour.h"

/*
#define HEUR
#define NODOTRAIN
#define PRINT_IT
#define SHARKIE
#define MINPRINT
#define CHECKTHECONS
#define SPECIAL0
#define SPECIAL3
 */
#define CHOOSE_SACRED
#define SPECIAL2
#define DECREASE

#include "stdmacro.h"
#include "disperse.h"
#include "matrix.h"
#include "onetreeq.h"
#include <assert.h>
#include "io.h"
#include "params.h"

class chead_t {
public:
   short id, head, trvd;
};

class Label_t {
public:
   city_id_t label, second_edge, third_edge;
};

static Label_t st_labels[MAX_DEGREE];

#ifdef HEUR
#define order(_a,_b) ((void)(_b), order(_a, 0))
#endif

static city_t* micmap[MAX_DEGREE];

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
#define OPT_DIFF_BREAKS ((int)sm->degree/2)
 */
#define OPT_DIFF_BREAKS ((int)sm->degree*4)

/* This is the iterations of the initial ascent */
#define INITIAL_MAX_ITERATIONS ((int)(sm->degree*sm->degree)/50+16+sm->degree)
/*
#define INITIAL_MAX_ITERATIONS 4000
#define INITIAL_MAX_ITERATIONS 100000
 */
/* This is the iterations of the general ascent */
#define GENERAL_MAX_ITERATIONS ((int)(sm->degree-depth)/2+2)

inline int OneTree :: will_train(const sum_t most)
{
#ifdef NODOTRAIN
   return 0;
#else
   /*
   return 0;
       */
   return (most - optimal) >
      (double)(1./100.)*((most - solver->Initial_Lower_Bound));
   /*
   return 1;
    */
#endif
};

inline int OneTree :: stat_will_train(const sum_t most)
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
   st_ostats.dist[index]++;
   return will_train(most);
}

void OneTree :: trip(
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
   short val;
   inline marker_t () { val = 0; };
   inline short is_true() { return val; };
   inline short is_false() { return (short)!val; };
   inline void make_true() { val = 1; };
   inline void not() { val = (short)!val; };
};

// first print the partial tours spearated by commas and print the
// cost it takes to travel from one site to another in the partial tours
// surrounded by '<' '>' characters.   Then print the one tree part
// as each city id and it's corresponding parent with the city who
// has no parent being a root of one of the possible one-trees, if the
// onetreee is complete it will only have one root.  finaly the last
// edge of the onetree is printed, the edge that separates the onetree from
// a minmums spaninng tree
void OneTree :: print() const
{
   city_t *city, *site;
   marker_t *marker = new marker_t[sm->degree];
   city_id_t id, save, last;

   dump.form("D%dO%-5d", (int)depth, (int)optimal);

   for (city = section2; city < section3; city++) {
      assert(city->section == 2);
      if (marker[city->id].is_false()) {
         last = city->id;
         id = city->u.sec2.required_edge;
         dump << city->id << "<";
         if (param.verbose > 2)
            dump << (long)city->find_cost(id, sm);
         dump << ">";
         marker[city->id].make_true();
         for (;;) {
            assert(marker[id].is_false());
            marker[id].make_true();
            for (site = mic_list; site < section3; site++)
               if (site->id == id)
                  break;
            assert(site != section3);
            if (site >= section2)
               break;
            save = last;
            last = id;
            id = site->up(save);
            dump << site->id << "<";
            if (param.verbose > 2)
               dump << (long)site->find_cost(id, sm);
            dump << ">";
         }
         assert(site->section == 2);
         dump << id << ",";
      }
   }
   dump << "*,";
   for (city = mic_list; city < section2; city++) {
      assert(city->section == 1);
      assert(marker[city->id].is_true());
   }
   for (site = NULL, city = section2; city < end_city; city++) {
      if (city->id == sacred_id)
         site = city;
      dump << city->id << "(";
      if (city->up() == NO_ID)
         dump << "RT";
      else {
         dump << "[" << city->up() << "]";
         if (param.verbose > 2)
            dump << (long)city->find_cost(city->up(), sm);
      }
      dump << "),";
   }
   assert(site != NULL);
   dump << site->id << "([" << sacred_edge << "]";
   if (param.verbose > 2)
      dump << (long)site->find_cost(sacred_edge, sm);
   dump << ")\n";
   delete marker;
}

#ifdef CHECKTHECONS
/* Point *1*: ignore onetree subproblem check if it was meant to just crash and
 * die as an optimal MAX_SUM.
 */
void OneTree :: check_connections()
{
   marker_t *marker;
   city_t *city;
   short er = 0;
   city_id_t id, last, save;
   int num_branches = 0;

   if (optimal == MAX_SUM) /*1*/
      return;
   marker = new marker_t[sm->degree];
   for (city = mic_list; city < end_city; city++)
      micmap[city->id] = city;
   for (city = mic_list; city < section2; city++) {
      marker[city->id].val = 2;
      assert(city->section == 1);
      num_branches += 2;
   }
   for(; city < end_city; city++) {
      if (city < section3) {
         assert(city->section == 2);
         num_branches++;
         marker[city->id].val++;
         last = city->id;
         id = city->u.sec2.required_edge;
         while (micmap[id]->section != 2) {
            assert(micmap[id] < section2);
            save = last;
            last = id;
            id = micmap[id]->up(save);
         }
         assert(city->u.sec2.end == id && micmap[id]->u.sec2.end == city->id);
      }
      if (city->up() != NO_ID) {
         num_branches+=2;
         marker[city->up()].val++;
         marker[city->id].val++;
      }
   }
   assert(num_branches == 2*sm->degree-2);
   marker[sacred_id].val++;
   marker[sacred_edge].val++;
   for (city = mic_list; city < section2; city++) {
      if (marker[city->id].val != 2) {
         dump << "([" << city->id << "]" << marker[city->id].val << "<>2) ";
         er = 1;
      }
      if (city->branches != 2) {
         dump << "([+" << city->id << "+]" << city->branches << "<>2) ";
         er = 1;
      }
   }
   for(; city < end_city; city++) {
      if (marker[city->id].val != city->branches) {
         dump << "([" << city->id << "]" << marker[city->id].val << "<>"
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
inline void OneTree :: check_connections()
{
}
#endif

// C e H
inline void OneTree :: operator = (const OneTree &parent)
{
   if (sm != parent.sm)
      *sm = *parent.sm;
   depth = parent.depth;
   version = parent.version;
   optimal = parent.optimal;
   lambda_sum = parent.lambda_sum;
   memcpy(mic_list, parent.mic_list, sizeof(city_t)*sm->degree);
   ord = parent.ord;
   section2 = mic_list + (city_id_t)(parent.section2-parent.mic_list);
   section3 = mic_list + (city_id_t)(parent.section3-parent.mic_list);
   sacred_edge = parent.sacred_edge;
   sacred_id = parent.sacred_id;
   required_cost = parent.required_cost;
}

void OneTree :: heuristic_tour(sum_t &opt)
{
   city_id_t x, y, root=NO_ID;
   chead_t *ups = new chead_t[sm->degree];
   city_t *city, *end;
   CircList tourlist;
   TourListElement *t, *found_root = NULL;
   Path *p;
   TourIter ti;

   for (city = mic_list, end = city + sm->degree; city<end; city++) {
      assert(city->section == 3);
      if (city->u.sec3.onetree_edge == NO_ID)
         root = city->id;
      ups[city->id].trvd = 0;
      ups[city->id].id = city->u.sec3.onetree_edge;
      ups[city->id].head = (city_id_t) (city->branches != 2
         || city->id == sacred_edge || city->id == sacred_id);
   }
   assert(root != NO_ID);
   for (x = 0; x < sm->degree; x++) {
      if (ups[x].head) {
         assert(!ups[x].trvd);
         y = x;
         t = new TourListElement;
         tourlist.insert(t);
         ups[y].trvd = 1;
         t->tour.travel(y);
#ifdef SHARKIE
dump << y << ",";
#endif
         if (y != root)
         for (y = ups[y].id; !ups[y].head; y = ups[y].id) {
            if (y != root) {
               ups[y].trvd = 1;
               t->tour.travel(y);
#ifdef SHARKIE
dump << y << ",";
#endif
            }
            else { /* y==root */
               ups[y].trvd = 1;
               if (found_root==NULL) {
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
   delete ups;
   DispersionHeuristic d(solver->matrix, tourlist);
   /*
   Tour Trour(solver->matrix->degree);
   min_matching(&Trour);
    */
   while (d.run())
      ;
   CircListIter ci(tourlist);
   while ((t = (TourListElement*)ci.next()) != NULL)
      delete t;
   if (sm->degree > 15) {
      FindTour to("+ro+th+ro", solver->matrix, &solver->duration, 0, d.tour);
      /*
      FindTour toc("+ro+th+ro", solver->matrix, &solver->duration, 0, &Trour);
       */
      if (param.verbose > 1)
         dump << "Disp=" << d.length()
            /*
            << " Chist=" << Trour.cost(solver->matrix)
             */
            << " 3-Opt=" << to.tour->cost(solver->matrix)
            /*
            << "," << toc.tour->cost(solver->matrix)
             */
            << "\n";
      if (to.tour->cost(solver->matrix) < opt) {
         opt = to.tour->cost(solver->matrix);
         solver->tour->copy(*to.tour);
      }
   }
   else if (d.tour->cost(solver->matrix) < opt) {
      opt = d.tour->cost(solver->matrix);
      solver->tour->copy(*d.tour);
   }
}

void OneTree :: construct_tour(Tour *tour)
{
   city_id_t last_id, id, save, count = sm->degree, sedge = NO_ID;
   long state, section2s;
   city_t *city;

   trip("CNS ");
   for (city = mic_list; city < end_city; city++) {
      micmap[city->id] = city;
      if (city->branches == 1 && city->id != sacred_id)
         sedge = city->id;
   }
   if (c_factor() == 0) {
      assert(sedge == NO_ID);
      sedge = sacred_edge;
   }
   if (c_factor() > 2) {
      dump << "Bad C FACTOR SEED=" << (param.seed - 1) << "\n";
      return;
   }
   state = 1;
   section2s = 0;
   last_id = NO_ID;
   city = micmap[sacred_id];
   id = city->id;
   for(; count > 0; assert(id != NO_ID), city = micmap[id], count--) {
      if (state)
         tour->travel(id);
      else
         tour->insert_after(NULL, id);
      switch (city->section) {
      case 3:
         if (city->u.sec3.onetree_edge == NO_ID) {
            state--;
            assert(!state);
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
         assert(city->section==2);
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
            assert (!state);
            last_id = sacred_id;
            id = sedge;
         }
         break;
      }
   }
   if (sm->degree > 10) {
      FindTour to("+ro+th+ro", solver->matrix, &solver->duration, 0, tour);
      tour->copy(*to.tour);
   }
   optimal = tour->cost(solver->matrix);
}

// recalculate/find the sacred edge to the onetree
// this can be called anytime that the sacred edge is not connected.
//
// a micmap must be built before this is called.
//
// this is a slow method because it needs the second_edge connector so
// if all the 1 branched cities don't have up pointers, at least one of them
// needs to find it's up pointer from the second_edge list
//
// Point *1*: last_x can only be NO_ID if the city->id with one branch is at
// the end of a partial tour and has no ontree_edge pointing to it.
void OneTree :: sacred_span()
{
   lambda_cost_t max_cost = (lambda_cost_t)MIN_COST;
   city_t *city;
   SortedCost *low_base, *lb;

   low_base = lb = NULL;
   for (city = section2; city < end_city; city++) {
      if (city->branches == 1) {
         for (lb = sm->cost[city->id]; ; lb++) {
            if (city->up() != lb->id
             && !(city < section3 && lb->id == city->u.sec2.required_edge)) {
               city_t *c;
               for (c = mic_list; c < section2; c++)
                  if (c->id == lb->id)
                     break;
               if (c == section2)
                  break;
            }
            assert(lb < sm->cost[city->id]+sm->degree);
         }
         if (lb->cost > max_cost) {
            low_base = lb;
            max_cost = low_base->cost;
            sacred_id = city->id;
            sacred_edge = low_base->id;
         }
      }
   }
   if (low_base == NULL || low_base->real_cost == MAX_COST) {
      optimal = MAX_SUM;
      return;
   }
   micmap[sacred_id]->branches++;
   micmap[sacred_edge]->branches++;
   optimal += low_base->real_cost;
   lambda_sum += sm->lambdas[sacred_id] + sm->lambdas[sacred_edge];
}

/* Point *1*: The lambdas associated with the cities already traveled don't
 * affect lambda training, but do get lambda addition.
 *
 */
inline void OneTree :: sum_lambdas()
{
   city_t *city;

   lambda_sum = (lambda_sum_t)0;
   for (city = section2; city < end_city; city++)
      lambda_sum += sm->lambdas[city->id]*(lambda_sum_t)((int)city->branches-2);
   optimal += (sum_t)lambda_sum;
}

// compute the minimum spanning tree assuming you have forced edges
// in sections 1 and 2, and then compute the sacred onetree edge to
// make it a complete relaxation
//
// Point *1*: we call sacred_span and this adds to the lambda_sum but
// the lambda sum is overritten in sum_lambdas();
//
// Point *2*: here we label the first city traveled to start the
// minimum spaning tree algorithm.  If there are no section3 cities
// to mark then we take the first section2 pair and label both as
// being the first min_span cities.
void OneTree :: min_span()
{
   city_id_t last_x=NO_ID;
   short times, end_times;
   short set=1;
   SortedCost *base, *low_base=NULL, *endbase;
   lambda_cost_t lowest_cost;
   city_t *city;
   Label_t *labels=st_labels;

#ifdef PRINT_IT
   if (param.verbose > 3) {
      sm->print();
      solver->matrix->print();
   }
#endif
   for(city = mic_list; city < section2; city++) {
      labels[city->id].label = 2;
      micmap[city->id] = city;
   }
   for(; city < end_city; city++) {
      st_labels[city->id].label = 0;
      micmap[city->id] = city;
      city->reset();
   }
   optimal = required_cost;
   if (section3 < end_city) /*2*/
      labels[section3->id].label = 1;
   else {
      assert(section2 < end_city);
      labels[section2->id].label = 1;
      labels[section2->u.sec2.end].label = 1;
   }
   end_times = (short)((end_city-section3) + (short)(section3-section2)/2 - 1);
   for (times = 0; times < end_times; times++) {
      lowest_cost = MAX_LCOST;
      for (city = section2; city < end_city; city++) {
         if (labels[city->id].label == 1) {
            base = sm->cost[city->id];
            for (endbase = base+sm->degree-1; base < endbase; base++) {
               if (labels[base->id].label == 0) {
                  /*
                  if (base->cost < lowest_cost) {
                   */
                  if (base->cost <= lowest_cost) {
                     if (base->cost != lowest_cost
                      || base->real_cost < low_base->real_cost) {
                        low_base = base;
                        lowest_cost = low_base->cost;
                        last_x = city->id;
                     }
                  }
                  break;
               }
            }
         }
      }
      /*NOTE:1*/
      assert(lowest_cost != MAX_LCOST);
      if (set) {
         labels[low_base->id].label = 1;
         city = micmap[low_base->id];
         if (city->section == 2) {
            labels[city->u.sec2.end].label = 1;
            city->u.sec2.onetree_edge = last_x;
         }
         else
            city->u.sec3.onetree_edge = last_x;
         city->branches++;
         micmap[last_x]->branches++;
         if (times > end_times/2) {
            set = 0;
            for (city = section2; city < end_city; city++)
               labels[city->id].label =
                  (city_id_t)(labels[city->id].label ? 0 : 1);
         }
      }
      else {
         labels[last_x].label = 0;
         city = micmap[last_x];
         city->up() = low_base->id;
         if (city->section == 2) {
            labels[city->u.sec2.end].label = 0;
            city->u.sec2.onetree_edge = low_base->id;
         }
         else
            city->u.sec3.onetree_edge = low_base->id;
         city->branches++;
         micmap[low_base->id]->branches++;
      }
      optimal += low_base->real_cost; // !cut_edge
   }
   sacred_span(); /*1*/
   sum_lambdas();
   /*
   trip("SPAN");
    */
   check_connections();
}

long OneTree :: c_factor()
{
   long connection_factor = 0;
   city_t *city;

   for (city = section2; city < end_city; city++)
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
   int brnches;
} conn_t;

static conn_t *st_branches;

int OneTree :: sort_lambda_costs(const sum_t optimost,
#ifdef SPECIAL3
   int general)
#else
   int)
#endif
{
   city_t *city;
   long connection_factor = c_factor();
   float TT;
   int brnches;

   if (optimost<=optimal)
      return 2;
   if (connection_factor == 0)
      return 1;

#ifdef SPECIAL0
   TT = alpha*((float)(optimost - optimal)) / connection_factor;
#endif
#if defined(SPECIAL1) || defined (SPECIAL2)
   TT = alpha;
   alpha *= alpha_factor;
#endif
#if defined(SPECIAL1) || defined (SPECIAL0)
   for (city = section2; city < end_city; city++) {
      brnches = ((int)city->branches - 2);
      sm->lambdas[city->id] += (lambda_t) (1.3*TT*brnches
         + 0.7*TT*st_branches[city->id].brnches);
      st_branches[city->id].brnches = brnches;
   }
#endif
#ifdef SPECIAL2
   for (city = section2; city < end_city; city++) {
      brnches = ((int)city->branches - 2);
      sm->lambdas[city->id] += (lambda_t) (1.2*TT*brnches
         + 0.7*TT*st_branches[city->id].brnches);
      st_branches[city->id].brnches = brnches;
   }
#endif
#ifdef SPECIAL3
   if (general) {
      TT = alpha*((float)(optimost - optimal)) / (connection_factor / 2.);
      for (city = section2; city < end_city; city++) {
         brnches = ((int)city->branches - 2);
         sm->lambdas[city->id] += (lambda_t) (0.7*TT*brnches
            + 0.3*TT*st_branches[city->id].brnches);
         st_branches[city->id].brnches = brnches;
      }
   }
   else {
      TT = alpha;
      alpha *= alpha_factor;
   
      for (city = section2; city < end_city; city++) {
         brnches = ((int)city->branches - 2);
         sm->lambdas[city->id] += (lambda_t) (1.2*TT*brnches
            + 0.7*TT*st_branches[city->id].brnches);
         st_branches[city->id].brnches = brnches;
      }
   }
#endif

   sm->sort(solver->matrix, sm->lambdas);
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
#define RAND(max) (((unsigned long)rand()/(double)MAX_RAND)*(max))
int OneTree :: train_lambdas(sum_t optimost, int max_iters)
{
   int count, iters;
   int non_improvements = 0, is_tour = 0;
   OneTree *current_node;
   SortedMatrix sorted_matrix(*sm);
   int general = (max_iters == GENERAL_MAX_ITERATIONS);

   st_branches = new conn_t[sm->degree];
   if (general && version) {
      max_iters = INITIAL_MAX_ITERATIONS;
      for (count = 0; count < (int)sm->degree; count++)
         sorted_matrix.lambdas[count] = 0;
      dump << depth << " Level\n";
   }
#ifdef SPECIAL0
   alpha = START_ALPHA;
   alpha_factor = ALPHA_FACTOR(START_ALPHA, END_ALPHA, 3*max_iters/4.);
#endif
#ifdef SPECIAL2
   double connection_factor = c_factor();
   alpha = connection_factor ? (optimost-optimal)/connection_factor : 0.;
   alpha_factor = .99;
#endif
#ifdef SPECIAL3
   if (general) {
      alpha = START_ALPHA;
      alpha_factor = ALPHA_FACTOR(START_ALPHA, END_ALPHA, (3*max_iters/4.));
   }
   else {
      double connection_factor = c_factor();
      alpha = connection_factor ? (optimost-optimal)/connection_factor : 0.;
      alpha_factor = .99;
   }
#endif
#if defined(SPECIAL2) || defined(SPECIAL3)
   /*
   double j = 0;
   for (count = 0; count < (int)sm->degree; count++)
      j += fabs(sorted_matrix.lambdas[count]);
   j /= sm->degree;
    */
   for (count = 0; count < (int)sm->degree; count++) {
      /*
      sorted_matrix.lambdas[count] *= (.70+RAND(.50));
      sorted_matrix.lambdas[count] += (-j/2.+RAND(j));
       */
      st_branches[count].brnches = 0;
   }
#else
   for (count = 0; count < (int)sm->degree; count++)
      st_branches[count].brnches = 0;
#endif
   count = iters = 0;
   min_span();
   if (param.verbose > 1 && (!general || version)) {
      dump.form("First 1-Tree = %d\n", (int)optimal);
      dump.flush();
   }
   current_node = new OneTree(*this, &sorted_matrix);
   do {
      if ((is_tour = current_node->sort_lambda_costs(optimost, general)) != 0) {
         if (is_tour != 1)
            is_tour = 0;
         else {
            *this = *current_node;       /*1*/
         }
         break;
      }
      current_node->min_span();
      if (current_node->optimal > optimal) {
         if (param.verbose > 1 && (!general || version)) {
            if (param.verbose > 2)
               current_node->print();
            dump << "Tree " << iters << "::" << current_node->optimal
               << "(" << c_factor() << ") " << count
               << "\n";
         }
         if (current_node->optimal > optimal + (sum_t)OPT_DIFF_THRESHOLD)
            non_improvements=0;
         *this = *current_node;
         count=iters;
      }
      else {
         if (non_improvements++ > OPT_DIFF_BREAKS)
            break;
#if defined(DECREASE) && (defined(SPECIAL0) || defined(SPECIAL2))
/*
#ifdef SPECIAL2
         if (general)
#endif
 */
         alpha *= alpha_factor;
         if (param.verbose > 2 && (!general || version))
            dump << "Alpha " << alpha << "\n";
#endif
      }
   } while (iters++<max_iters+1);

   if (current_node->optimal > optimal) {
      *this = *current_node;
      count=iters;
      assert(0);
   }
   /*
   sm->sort(solver->matrix, sm->lambdas);
    */
   if (param.verbose > 1 && (!general || version)) {
      dump << "FINAL LAMBDA SUM " << lambda_sum << "\n";
      dump << "ITERS " << iters << " MAX " << max_iters << " highest_opt ->["
       << optimal << "]<- Used at iters " << count << "\n";
      if (param.verbose > 2)
         sm->print();
      dump.flush();
   }
   current_node->sm = NULL; // to stop unusing the matrix
#if defined(HEUR)
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
      solver->tour->copy(*to.tour);
   }
#endif
   delete current_node;
   delete st_branches;
   check_connections();
   if (version)
      version = 0;
   return is_tour;
}

inline void OneTree :: init_first_constructor(city_id_t init_id,
   SortedMatrix *m, TSPSolver *s)
{
   city_id_t x;
   city_t *ml;

   version = 0;
   sm = m;
   solver = (OneTreeSolver*)s;
   install_blocked_mallocs(m->degree*sizeof(city_t), 100);
   ml = section2 = section3 = mic_list = new city_t[m->degree];
   end_city = ml + sm->degree;
   if (init_id != NO_ID) {
      ml->id = init_id;
      for(x=0; x<m->degree; x++) {
         //sm->lambdas[x] = (lambda_t)0; // Already Done in sort(Matrix)
         if (x != init_id) {
            ml++;
            ml->id = x;
         }
      }
      depth = 0;
      required_cost = 0;
   }
}

/* find a minimum spanning tree for all the nodes except the init_id and
 * make the two lowest costs from init_id be the one-tree definition edges.
 *
 * The way you do this, is to go down all the untraveled sites looking for
 * its lowest edges that go to traveled sites.  Once you find the lowest one,
 * you add that to the minimum spanning tree and then  start over untill you
 * have no more un-traveled sites, but then you'd have N edges when you should
 * only have N-1 edges?
 */
OneTree :: OneTree (city_id_t init_id, SortedMatrix *m, TSPSolver *s,
   sum_t &opt)
{
   init_first_constructor(init_id, m, s);
#ifdef NODOTRAIN
   min_span();
#else
   if (train_lambdas(opt, INITIAL_MAX_ITERATIONS))
      depth = (city_id_t)(sm->degree-2);
   else
#endif
   /*
   else if (param.most == MAX_SUM)
    */
   if (sm->degree > 15)
      heuristic_tour(opt);

   order(optimal, depth);
   assert(sm->lambdas != NULL);
}

OneTree :: OneTree (SortedMatrix *m, TSPSolver *s, Tour *tour)
{
   init_first_constructor(0, m, s);
   min_span();
   order(optimal, depth);
   min_matching(tour);
}

OneTree :: OneTree (city_id_t init_id, SortedMatrix *m, TSPSolver *s)
{
   init_first_constructor(init_id, m, s);
   min_span();
   order(optimal, depth);
}

OneTree :: OneTree (SortedMatrix *m, TSPSolver *s)
{
   init_first_constructor(NO_ID, m, s);
   optimal = 0;
   order(optimal, depth);
}

/* make_root ensures the mic_list who has span_id == id sindex to NO_ID and
 * will be the root of the tree it exists on after the function is called.
 * It does this by following its existing sindex pointer untill it finds
 * the current root, and along the way it reverses the sindex pointers
 * to point towards the id rather than towards the direction it is following.
 * If micmap[id]->sindex is already NO_ID then it is already the root of
 * its tree and nothing id done.
 */
void OneTree :: make_root(city_id_t id)
{
   city_id_t new_id, *lastp, old_id=NO_ID;
   city_t *city;

   lastp = &(city=micmap[id])->up();
   if (*lastp != NO_ID)
      new_id = *lastp;
   else if (city->section == 2
    && (new_id = *(lastp=&micmap[id=city->u.sec2.end]->up())) != NO_ID)
      ;
   else
      return;
   for (;;) {
      *lastp = old_id;
      old_id = id;
      id = new_id;

      lastp = &(city=micmap[id])->up();
      if (*lastp != NO_ID)
         new_id = *lastp;
      else if (city->section == 2 && micmap[city->u.sec2.end]->up() != NO_ID) {
         id = NO_ID;
         new_id = city->u.sec2.end;
      }
      else
         break;
   }
   *lastp = old_id;
}

/* this labels the st_labels list with which id belongs to which tree.
 * the "label" passed is the label that should be marked for ids
 * with out a tree.
 *
 * Point *2*: we choose the first tree as the first section2 partial tour id.
 * This is i believe a garuanteed tree always in the onetree.  and will never
 * be == end_city
 */
city_id_t OneTree :: Label(trees_t *trees)
{
   register city_id_t y, ny;
   register tree_t *tr=trees->trees;
   register city_id_t lab;
   Label_t *lbl, *end;
   city_t *city;

   trees->num_trees = 0;
   assert(section2 != end_city);
   tr->label = NO_ID;
   lab = section2->tree_id(); /*2*/
   while ((ny=micmap[lab]->travel_up((const city_t **)micmap)) != NO_ID)
      lab = ny;
   assert(lab != NO_ID);
   for (lbl=st_labels, end = lbl+sm->degree; lbl<end; lbl++)
      lbl->label = lab;
   for (city = section2; city < end_city; city++) {
      y = city->tree_id();
      while ((ny=micmap[y]->travel_up((const city_t **)micmap)) != NO_ID) {
         if ((y = st_labels[y].label) != lab)
            break;
         y = ny;
      }
      if (!(tr->label == y)) {
         for (tr=trees->trees; tr < trees->trees+trees->num_trees; tr++) {
            if (tr->label == y)
               break;
         }
         if (!(tr < trees->trees+trees->num_trees)) {
            tr = trees->trees + trees->num_trees++;
            tr->label = y;
            tr->edges = 1;
         }
      }
      else {
         tr->edges++;
      }
      st_labels[city->id].label = y;
      if (city < section3)
         st_labels[city->u.sec2.end].label = y;
   }
   return lab;
}

/* this procedure is called after all cuts are made from the onetree,
 * and after this is called a new minimum spaning tree will be mended.
 *
 * We always assume the micmap has been created before inc_min_span is called.
 * The cuts will assumed to have been excluded edges or hidden edges
 * from requiring an edge.  the a_root passed needs to be a root for
 * one of the trees that needs to be mended.
 */
inline void OneTree :: incremental_min_span()
{
   city_t *city, *last_city=NULL;
   static trees_t trees;
   city_id_t the_tree;
   city_id_t newtree;
   SortedCost *base, *low_base=NULL, *end;
   lambda_cost_t lowest_cost;

   the_tree = Label(&trees);
   lowest_cost = MAX_LCOST;
   for (trees.num_trees--; trees.num_trees > 0; trees.num_trees--) {
      if (lowest_cost != MAX_LCOST) {
         newtree = st_labels[last_city->up()].label;
         for (city = section2; city < end_city; city++)
            if (st_labels[city->id].label == newtree)
               st_labels[city->id].label = the_tree;
      }
      lowest_cost = MAX_LCOST;
      for (city = section2; city < end_city; city++) {
         if (st_labels[city->id].label == the_tree) {
            base = sm->cost[city->id];
            for (end = base+sm->degree; ; base++) {
               assert(base < end);
               if (st_labels[base->id].label != the_tree) {
                  if (base->cost < lowest_cost) {
                     lowest_cost = base->cost;
                     low_base = base;
                     last_city = city;
                  }
                  break;
               }
            }
         }
      }
      /*NOTE:1 assert(lowest_cost != MAX_LCOST); */
      if (lowest_cost == MAX_LCOST) { /*1*/
         optimal = MAX_SUM;
         return;
      }
      make_root(last_city->id);
      last_city->up() = low_base->id;
      optimal += low_base->real_cost; // !cut_edge
      lambda_sum += sm->lambdas[last_city->id] + sm->lambdas[low_base->id];
      last_city->branches++;
      micmap[low_base->id]->branches++;
   }
   sacred_span();
}

void OneTree :: construct (const OneTree &parent)
{
   solver = parent.solver;
   sm = parent.sm;
   mic_list = new city_t[sm->degree];
   end_city = mic_list + sm->degree;
   *this = parent;
}

void OneTree :: cut_edge(const city_id_t a, const city_id_t b)
{
   optimal -= solver->matrix->val(a,b);
   lambda_sum -= sm->lambdas[a] + sm->lambdas[b];
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
// Point *3*: if cityb is the section ptr then the cityb will be move into
// the pointer space of citya.
//
void OneTree :: require_edge(city_id_t a, city_id_t b)
{
   city_t *citya=micmap[a], *cityb=micmap[b], *city;
   city_id_t up;

   required_cost += solver->matrix->val(a,b);
   if (citya->section == cityb->section) {
      if (citya->section == 2) { /*1*/
         assert(micmap[citya->u.sec2.end]->section == 2 &&
            micmap[citya->u.sec2.end]->u.sec2.end == citya->id);
         micmap[citya->u.sec2.end]->u.sec2.end = cityb->u.sec2.end;
         assert(micmap[cityb->u.sec2.end]->section == 2 &&
            micmap[cityb->u.sec2.end]->u.sec2.end == cityb->id);
         micmap[cityb->u.sec2.end]->u.sec2.end = citya->u.sec2.end;
         citya->branches = cityb->branches = 2;
         for (city = section2; city < end_city; city++) {
            up = city->up();
            if (up == a) {
               city->up() = NO_ID;
               if (city->id != b) {
                  cut_edge(city->id, a);
                  city->branches--;
               }
            }
            else if (up == b) {
               city->up() = NO_ID;
               if (city->id != a) {
                  cut_edge(city->id, b);
                  city->branches--;
               }
            }
         }
         if ((up = citya->up()) != NO_ID && up != b) {
            cut_edge(a, up);
            micmap[up]->branches--;
         }
         if ((up = cityb->up()) != NO_ID && up != a) {
            cut_edge(b, up);
            micmap[up]->branches--;
         }
         micmap[a] = section2;
         if (cityb == section2) /*3*/
            cityb = citya;
         citya->degrade_section(section2, b);
         micmap[citya->id] = citya;
         micmap[b] = section2;
         cityb->degrade_section(section2, a);
         micmap[cityb->id] = cityb;
      }
      else {
         micmap[a] = section3;
         if (cityb == section3) /*3*/
            cityb = citya;
         citya->degrade_section(section3, b, b);
         micmap[citya->id] = citya;
         if (micmap[a]->u.sec2.onetree_edge == b)
            micmap[a]->u.sec2.onetree_edge = NO_ID;
         micmap[b] = section3;
         cityb->degrade_section(section3, a, a);
         micmap[cityb->id] = cityb;
         if (micmap[b]->u.sec2.onetree_edge == a)
            micmap[b]->u.sec2.onetree_edge = NO_ID;
      }
   }
   else {
      if (citya->section > cityb->section) {
         city = cityb; cityb = citya; citya = city; up = b; b = a; a = up;
      }
      assert(citya->section == 2 && cityb->section == 3);
      assert(micmap[citya->u.sec2.end]->section == 2 &&
         micmap[citya->u.sec2.end]->u.sec2.end == citya->id);
      micmap[citya->u.sec2.end]->u.sec2.end = b;
      citya->branches = 2;
      for (city = section2; city < end_city; city++) {
         if (city->up() == a) {
            city->up() = NO_ID;
            if (city->id != b) {
               cut_edge(city->id, a);
               city->branches--;
            }
         }
      }
      if ((up = citya->up()) != NO_ID && up != b) {
         cut_edge(a, up);
         micmap[up]->branches--;
      }
      micmap[b] = section3;
      if (citya == section3)
         citya = cityb;
      cityb->degrade_section(section3, a, citya->u.sec2.end);
      micmap[cityb->id] = cityb;
      if (micmap[b]->u.sec2.onetree_edge == a)
         micmap[b]->u.sec2.onetree_edge = NO_ID;
      micmap[a] = section2;
      citya->degrade_section(section2, b);
      micmap[citya->id] = citya;
   }
}

void OneTree :: forbid_edge(const city_id_t a, const city_id_t b)
{
   city_t *citya=micmap[a], *cityb=micmap[b];

   cut_edge(a,b);
   citya->branches--;
   cityb->branches--;
   smid = sm->new_smid();
   sm->forbid_edge(smid, a, b);
   sm->forbid_edge(smid, b, a);
   if (citya->up() == b)
      citya->up() = NO_ID;
   else if (sacred_id != a || sacred_edge != b) {
      assert(cityb->up() == a);
      cityb->up() = NO_ID;
   }
}

// is passed a boolean to cut or not cut the sacred edge out
void OneTree :: incremental_begin(const int cut_sacred)
{
   city_t *city;
   trip("Bef ");
   optimal -= (sum_t)lambda_sum;
   for (city=section2; city < end_city; city++)
      micmap[city->id] = city;
   if (cut_sacred) {
      cut_edge(sacred_id,  sacred_edge);
      micmap[sacred_id]->branches--;
      micmap[sacred_edge]->branches--;
   }
}

void OneTree :: incremental_end()
{
   incremental_min_span();
   check_connections();
   optimal += (sum_t)lambda_sum;
   trip("Aft ");
}

OneTree :: OneTree (const OneTree &parent, const city_id_t a, const city_id_t b,
   const city_id_t c)
{
   construct(parent);
   incremental_begin(1);

   require_edge(a,b);
   require_edge(b,c);
   trip("RR  ");

   incremental_end();
   order(max(optimal, ord.sum), depth+=(city_id_t)2);
}

OneTree :: OneTree (const OneTree &parent, const city_id_t a, const city_id_t b)
{
   construct(parent);
   incremental_begin(sacred_id != a || sacred_edge != b); /*2*/

   forbid_edge(a, b);
   trip("F   ");

   incremental_end();
   order(max(optimal, ord.sum), depth);
}

/* Point *1*: the require_edge() should happen after the forbid edge since
 * the if require happens first it might move around the miclist under the
 * b the pivot point would be dangling.  Note: this does not happen when
 * we don't use a pivot pointer and just use the id 'b' like we are doing now.
 *
 * Point *2*: if the edge being forbiden is the sacred edge we should
 * not cut it in incremental_begin, and assert the right side is being
 * cut if so.
 */
OneTree * OneTree :: rebirth(const city_id_t a, const city_id_t b,
   const city_id_t c)
{
   incremental_begin(sacred_id != a || sacred_edge != b); /*2*/
   forbid_edge(a, b); /*1*/
   trip("RF0 ");
   require_edge(b, c);
   trip("RF  ");
   incremental_end();
   order(max(optimal, ord.sum), depth+=(city_id_t)1);
   return this;
}

OneTree :: OneTree (const OneTree &parent)
{
   construct(parent);
}

OneTree :: OneTree (const OneTree &parent, SortedMatrix *new_sm)
{
   solver = parent.solver;
   sm = new_sm;
   mic_list = new city_t[sm->degree];
   end_city = mic_list + sm->degree;
   *this = parent;
}

onetree_stats st_ostats;

void init_onetree_stats()
{
   short x;
   st_ostats.Trains = st_ostats.Explores = 0;
   for (x = 0; x < MAX_OT_STATS; x++) {
      st_ostats.dist[x] = 0;
   }
}

void free_onetree_stats(Term &term)
{
   short x;
   term << "Ostats : ";
   term << "Trained " << ((st_ostats.Explores == 0) ? (double)0.0 :
      (double)st_ostats.Trains*(double)100.0/(double)st_ostats.Explores)
      << "% of " << (double)st_ostats.Explores << " Explores\n";
   for (x = 0; x < MAX_OT_STATS; x++) {
      if (st_ostats.dist[x] != 0) {
         term << x << "%==" << st_ostats.dist[x] << ", ";
      }
   }
   term << "\n";
}

// Try to train this subproblem
int OneTree :: try_train(const sum_t most)
{
   if (stat_will_train(most)) {
      st_ostats.Trains++;
      sm->sort(solver->matrix, sm->lambdas);
      if (param.verbose > 2)
         dump << "TRAIN";
      /*
      make_lone_matrix()
       */
      solver->diff_total -= optimal;
      if (train_lambdas(most, GENERAL_MAX_ITERATIONS)) {
         solver->diff_total += optimal;
         return 1;
      }
      solver->diff_total += optimal;
      solver->diffs++;
   }
   return 0;
}

/* Point *1*: besure to go backwards to try to get a section 2 city more
 * than a section 3 city.
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
 * Point *3*: if savecity is section2 city that is root and has sacred edge
 * pointing to it.
 *
 * Point *4*: if savecity has nothing ponting to it then it must have a
 * sacred edge off it.
 */
void OneTree :: branch(PQueue &que)
{
   SortedMatrix *savesm=sm, *currsm;
   city_t *city, *savecity = NULL;
   city_id_t x, first = 0, second;
   OneTree *new_node;

   trip("BRC ");
   if (try_train(que.most)) {
      st_ostats.Explores++;
      depth = (city_id_t)(sm->degree-1);
      order(max(optimal, ord.sum), depth);
      if (!que.enq(this))
         unuse_matrix(savesm);
      que.dont_delete_last_dequeued();
      return;
   }
   for (x=0; x < savesm->degree; x++)
      st_labels[x].second_edge = st_labels[x].third_edge = NO_ID;
   for (city = end_city-1; city >= section2; city--) { /*1*/
      if ((x = city->up()) != NO_ID) { /*PATCH*/
         if (st_labels[x].second_edge == NO_ID)
            st_labels[x].second_edge = city->id;
         else
            st_labels[x].third_edge = city->id;
      }
      if (city->branches > 2) {
#ifdef CHOOSE_SACRED
         savecity = city;
#else
         if (savecity == NULL || first) {
            savecity = city;
            assert(savecity->id != sacred_id);
            first = (savecity->id == sacred_id
             || savecity->id == sacred_edge);
         }
#endif
      }
   }
   if ((city=savecity) == NULL) {
      st_ostats.Explores++;
      depth = (city_id_t)(sm->degree-1);
      order(max(optimal, ord.sum), depth);
      if (!que.enq(this))
         unuse_matrix(savesm);
      que.dont_delete_last_dequeued();
      return;
   }
   if (city->up() == NO_ID && st_labels[city->id].third_edge == NO_ID) { /*3*/
      assert(city < section3 && sacred_edge == city->id);
      second = sacred_id;
      first = st_labels[city->id].second_edge;
   }
   else if (st_labels[city->id].second_edge == NO_ID) { /*4*/
      assert(city < section3 && sacred_edge == city->id);
      second = sacred_id;
      first = city->up();
   }
   else {
      second = (st_labels[city->id].third_edge != NO_ID)
         ? st_labels[city->id].third_edge : city->up();
      first = st_labels[city->id].second_edge;
   }

   // branch by requiring two edges
   if (city >= section3) {
      use_matrix(sm);
      st_ostats.Explores++;
      new_node = new OneTree (*this, first, city->id, second);
      if (!que.enq( new_node ))
         unuse_matrix(savesm);
   }

   // branch by forbiding first
   currsm = sm = start_using_matrix(*savesm);
   st_ostats.Explores++;
   new_node = new OneTree (*this, first, city->id);
   if (!que.enq( new_node ))
      unuse_matrix(currsm);

   // branch by requireing first and forbiding second
   currsm = sm = start_using_matrix(*savesm);
   st_ostats.Explores++;
   new_node = rebirth (second, city->id, first);
   if (!que.enq( new_node ))
      unuse_matrix(currsm);
   que.dont_delete_last_dequeued();
   unuse_matrix(savesm);
}

void OneTree :: write(BinFile &t) const
{
   city_t *city;
#ifndef OLD_TIMER
   t << version;
#endif
   t << smid << sm << optimal << depth
      << required_cost << sacred_id << sacred_edge << lambda_sum
      << (section2-mic_list) << (section3-mic_list);
   for (city = mic_list; city<end_city; city++)
      city->write(t);
}

PQuelem *OneTree :: read_clone(BinFile &t, TSPSolver *s) const
{
   long sec2, sec3;
   void *smatrix;
   city_t *city;
   OneTree *o = new OneTree;

#ifndef OLD_TIMER
   t >> o->version;
#else
   o->version = 1;
#endif
   t >> o->smid >> smatrix >> o->optimal >> o->depth
      >> o->required_cost >> o->sacred_id >> o->sacred_edge >> o->lambda_sum
      >> sec2 >> sec3;
   o->sm = find_SortedMatrix_ptr(smatrix);
   o->solver = (OneTreeSolver*)s;
   o->mic_list = new city_t[o->sm->degree];
   o->end_city = o->mic_list+o->sm->degree;
   o->section2 = o->mic_list+(city_id_t)sec2;
   o->section3 = o->mic_list+(city_id_t)sec3;
   for (city = o->mic_list; city<o->end_city; city++)
      city->read(t);

   return o;
}

/* make subproblem by forcing a tour into sections 1 and 2, section two only
 * contains the outer cities of the subproblem tour, and section 1 contains the
 * guts.
 */
OneTree :: OneTree (const OneTree &parent, const city_id_t sub,
   const city_id_t k, Tour *t)
{
   city_id_t count, beg_city, sec_city;
   Path *p, *n, *c;
   city_t *ml;

   construct(parent);

   c = t->get_head();
   p = c->get_prev();
   for (count=0, n = c->get_next(); count<sub; count++) {
      p = c;
      n = (c = n)->get_next();
   }
   required_cost = solver->matrix->val(p->id, c->id);
   beg_city = p->id;
   sec_city = c->id;
   for (ml = mic_list, count = sm->degree; --count > k+1; ml++) {
      required_cost += solver->matrix->val(c->id, n->id);
      ml->u.sec1.required_edge1 = p->id;
      ml->u.sec1.required_edge2 = n->id;
      ml->id = c->id;
      ml->branches = 2;
      ml->section = 1;
      p = c;
      n = (c = n)->get_next();
   }
   section2 = ml;
   section3 = ml+2;
   ml->u.sec2.required_edge = p->id;
   ml->u.sec2.end = beg_city;
   ml->id = c->id;
   ml[1].id = beg_city;
   ml[1].u.sec2.required_edge = sec_city;
   ml[1].u.sec2.end = c->id;
   ml->branches = ml[1].branches = 1;
   ml->u.sec2.onetree_edge = ml[1].u.sec2.onetree_edge = NO_ID;
   ml->section = ml[1].section = 2;
   for (ml += 2, count--; count > 0; ml++, count--) {
      ml->id = n->id;
      n = n->get_next();
   }
   depth = sm->degree - k - 1;
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
void OneTree :: min_matching(Tour *ret)
{
   city_id_t cons=0, here[4];
   Matching *matchings, *con1, *con2, *con3, *conend, *best_con2;
   Matching *best_con3 = NULL;
   sum_t sofar, sofar_best, tmp, almost, almost_b2;
   cost_t *con1b2, *con1b1, *con2b2, *con2b1;
   Tour traveled(sm->degree);
   city_t *city;
   const Matrix *matrix = solver->matrix;

#ifdef NO_ARRAY_CONSTRUCTING
   assert(0);
#endif
   matchings = new Matching[sm->degree];
   for (city = section2; city < end_city; city++) {
      if ((city->branches
       - ((city->id == sacred_id || city->id == sacred_edge) ? 1 : 0)) % 2) {
         if (!(cons % 2))
            matchings[cons/2].b1 = city->id;
         else {
            con1 = matchings+(cons/2);
            con1->b2 = city->id;
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
   for (con1 = matchings, city = mic_list; city < end_city; city++) {
      city->place(matchings, con1);
   }
#ifdef MINPRINT
for (con1 = matchings; con1 < matchings+sm->degree; con1++) {
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
