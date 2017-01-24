/* onetquel.cc
 *
 * simple branching the 1-TREE que element
 *
 * orig ceh
 */

#include <math.h>

/*
#define PRINT_IT
 */
/* if LAMPATHCOST is defined, pathcost has lambdas in it
 * if not the path cost is the true cost of the traverse from city to city
#define LAMPATHCOST
#define CHECKTHECONS
 */

#include "onetquel.h"
#include <assert.h>
#include "io.h"
#include "params.h"

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
#define ALPHA_FACTOR(iters) \
   pow((double)END_ALPHA/START_ALPHA, (double)1.0/(iters))
#define DECREASE
/*
 */
/*
 */
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
#define OPT_DIFF_THRESHOLD 1.0
/*
#define OPT_DIFF_BREAKS (17)
 */
#define OPT_DIFF_BREAKS (sm->degree/2)

/* This is the iterations of the initial ascent */
#define INITIAL_MAX_ITERATIONS ((sm->degree*sm->degree)/10+16+sm->degree)
/*
#define INITIAL_MAX_ITERATIONS 1000
 */
/* This is the iterations of the general ascent */
#define GENERAL_MAX_ITERATIONS (5)

inline int onode_t :: will_train(const sum_t most)
{
   return (most - optimal) >
     (double)(10./100.)*((most-solver->Initial_Lower_Bound))
    ;
};

inline int onode_t :: stat_will_train(const sum_t most)
{
   const double p = most - optimal;
   const double max = most-solver->Initial_Lower_Bound;
   int index = (int)((100.0*p)/max);

   if (index >= 100 || index < 0) {
      assert(fabs(solver->Initial_Lower_Bound - optimal) <= SUM_GRANU);
      index = (index>=100) ? 100 : 0;
   }
   st_stats.dist[index]++;
   return will_train(most);
}

/* the maximum number of edges that can be coming from a minimum spanning
 * tree vertex.  Or the maximum number of trees that can be revealed in a
 * subproblem minimum spanning tree "cut".
 */
#define MAX_SPAN_CONNECTIONS (MAX_DEGREE/10+4)

/* the impeded specifications
 */
#define NOT_IMPEDED 1
#define IMPEDED 0
#define LAST_NOT_IMPEDED 2
#define IMP_MASK 0x3

void onode_t :: Print() const
{
   print();
}

void onode_t :: print() const
{
   city_id_t x;
   dump.form("D%-2d O%-6d ", (int)depth, (int)optimal);
#ifdef LAMPATHCOST
   dump.form("I(%d)%d+", (int)initcost->id, (int)initcost->cost);
#else
   dump.form("I(%d)%d+", (int)initcost->id, (int)initcost->real_cost);
#endif
   for (x=0; x<=depth; x++) {
      dump << "T" << (unsigned short)mic_list[x].span_id << "[";
      if (x==depth)
         dump << "(" << (unsigned short)one_tree_city << ")";
      dump << (unsigned short)mic_list[x].pathcost << "]";
   }
   for (; x<matrix->degree; x++) {
      if (mic_list[x].sindex == NO_ID)
         dump.form("%d[ROOT]%d,", (int)mic_list[x].span_id,
            (int)mic_list[x].brnchs);
      else
         dump.form("%d[(%d)%d]%d,", (int)mic_list[x].span_id,
            (int)mic_list[x].sindex,
            (int)matrix->val(mic_list[x].span_id,mic_list[x].sindex),
            (int)mic_list[x].brnchs);
   }
   dump << "\n";
}

#ifdef CHECKTHECONS
void onode_t :: check_connections()
{
   city_id_t x, *br = new city_id_t[sm->degree];
   short er=0;

   for (x=0; x<(city_id_t)(depth+1); x++)
      br[mic_list[x].span_id]=2;
   for(; x<sm->degree; x++)
      br[mic_list[x].span_id] = 0;
   for(x=(city_id_t)(depth+1); x<sm->degree; x++) {
      if (mic_list[x].sindex != NO_ID) {
         br[mic_list[x].sindex]++;
         br[mic_list[x].span_id]++;
      }
   }
   br[one_tree_city]++;
   br[initcost->id]++;
   for (x=0; x<(city_id_t)(depth+1); x++) {
      if (br[mic_list[x].span_id] != 2)
         er=1;
   }
   for (; x<sm->degree; x++) {
      if (br[mic_list[x].span_id] != (city_id_t)mic_list[x].brnchs)
         er=1;
   }
   if (er) {
      print();
      for (x=0; x<(city_id_t)(depth+1); x++) {
         dump << "[" << mic_list[x].span_id << "]";
         dump << "(" << br[mic_list[x].span_id] << "<>" << 2 << ")";
      }
      for (; x<sm->degree; x++) {
         dump << "[" << mic_list[x].span_id << "]";
         dump << "(" << br[mic_list[x].span_id]
            << "<>" << mic_list[x].brnchs << ")";
      }
      dump << "\n";
   }
   delete br;
}
#else
inline void onode_t :: check_connections()
{
}
#endif


typedef mic_listT_t *mic_ptr;
static mic_ptr micmap[MAX_DEGREE];

typedef struct Label_t {
   city_id_t label;
   _uchar traveled;
} Label_t;
static Label_t st_labels[MAX_DEGREE];

inline void onode_t :: operator = (const onode_t &parent)
{
  if (sm != parent.sm) {
     city_id_t init=parent.mic_list[0].span_id;
     *sm = *parent.sm;
     initcost = sm->cost[init]
        + (city_id_t)(parent.initcost - parent.sm->cost[init]);
  }
  else
     initcost = parent.initcost;
  depth = parent.depth;
  pure_depth = NO_ID;
  optimal = parent.optimal;
  lambda_sum = parent.lambda_sum;
  one_tree_city = parent.one_tree_city;
  memcpy(mic_list, parent.mic_list, sizeof(mic_listT_t)*matrix->degree);
  /*
  city_id_t x;
  for (x=0; x<matrix->degree; x++)
     mic_list[x] = parent.mic_list[x];
   */
}

/* Point *1*: must be able to construct a tour where both of the onetree
 * brnchs doesn't point to the root of the tree.  Therefore, start by
 * making a tour of mic_list[0].span_id through mic_list[pure_depth].span_id
 * then we follow mic_list[micmap[one_tree_city]->sindex].span_id until
 * we find the root.  then we follow and construct a list of cities with
 * st_labels, from * mic_list[micmap[initcost->id]->sindex].span_id
 * untill we find the root again, then we travel the list backwards
 * traveling cities and you should have a tour.
 */
void onode_t :: construct_tour(Tour *tour)
{
   city_id_t x, y, root;
   if (c_factor()!=0)
      dump << "Bad C FACTOR SEED=" << param.seed << "\n";
   if (pure_depth == NO_ID)
      for (x = 0; x < sm->degree; x++)
         tour->travel(mic_list[x].span_id);
   else {
      for (x=0; x < sm->degree; x++)
         micmap[mic_list[x].span_id] = mic_list+x;
      for (x = 0; x < (city_id_t)(pure_depth+1); x++) /*1*/
         tour->travel(mic_list[x].span_id);
      for (y=one_tree_city; root=y, (y=micmap[y]->sindex) != NO_ID; )
         tour->travel(root);
      tour->travel(root);
      for (y = initcost->id, x=0; y != root; x++, y=micmap[y]->sindex) {
         assert(x < (city_id_t)(sm->degree-pure_depth-1));
         st_labels[x].label = y;
      }
      while (x>0)
         tour->travel(st_labels[--x].label);
   }
}

/* min_span will compute the initial one_tree.  this has the stange
 * characteristic in the min span tree, that the ROOT will always be the
 * id that is traveled with the "2".
 * 
 * Point *1*: give the initial id its first and second lowest costs
 * keeping the second as the path, but the first as the back init ptr.
 *
 * Point *2*: this isn't done because you assume depth mic_lists are traveled
 * and already have two brnchs.
 */
void onode_t :: min_span()
{
   city_id_t x, last_x = NO_ID, a_sindex, degree=sm->degree, times;
   short *traveled = new short[degree];
   short set = 1, not_set = 0;
   SortedCost *base, *low_base = NULL, *initcost2;
   lambda_cost_t lowest_cost;

   if (param.verbose>2) {
      sm->print();
   }
   for(times=0; times<(city_id_t)(depth+1); times++) {
      traveled[mic_list[times].span_id] = 2;
      micmap[mic_list[times].span_id] = mic_list+times;
   }
   for(; times<degree; times++) {
      traveled[mic_list[times].span_id] = 0;
      micmap[mic_list[times].span_id] = mic_list+times;
      mic_list[times].sindex = NO_ID;
      mic_list[times].brnchs = 0;
   }
   /*1*/
   initcost = sm->cost[mic_list->span_id]+0;
   while (traveled[initcost->id])
      initcost++;
   initcost2 = sm->cost[mic_list[depth].span_id]+0;
   while (traveled[initcost2->id] || initcost == initcost2)
      initcost2++;
#ifdef LAMPATHCOST
   (mic_list+depth)->pathcost = initcost2->cost;
   optimal = (sum_t)(mic_list+depth)->pathcost + initcost->cost;
#else
   (mic_list+depth)->pathcost = initcost2->real_cost;
   optimal = (sum_t)(mic_list+depth)->pathcost + initcost->real_cost;
#endif
   one_tree_city = initcost2->id;
   for(times=0; times<depth; times++)
      optimal += mic_list[times].pathcost;
   micmap[initcost2->id]->brnchs++;
   micmap[initcost->id]->brnchs++;
   traveled[mic_list[depth+1].span_id] = 1;
   for (times=(city_id_t)(depth+2); times<degree; times++) {
      lowest_cost = MAX_LCOST;
      for (x=0; x<degree; x++) {
         if (traveled[x] == set) {
            base = sm->cost[x];
            for (a_sindex=0; a_sindex<(city_id_t)(degree-1); a_sindex++) {
               if (traveled[base[a_sindex].id] == not_set) {
                  if (base[a_sindex].cost < lowest_cost) {
                     lowest_cost = base[a_sindex].cost;
                     low_base = base+a_sindex;
                     last_x = x;
                  }
                  break;
               }
            }
         }
      }
      if (set) {
         traveled[low_base->id] = 1;
         micmap[low_base->id]->sindex = last_x;
         micmap[low_base->id]->brnchs++;
         micmap[ last_x ]->brnchs++;
      }
      else {
         traveled[last_x] = 1;
         micmap[last_x]->sindex = low_base->id;
         micmap[last_x]->brnchs++;
         micmap[ low_base->id ]->brnchs++;
      }
#ifdef LAMPATHCOST
      optimal += low_base->cost;
#else
      optimal += low_base->real_cost;
#endif
      if (times > (city_id_t)((degree-depth+2)/2)) {
         set = 0;
         not_set = 1;
      }
   }
   delete traveled;
   check_connections();
}

typedef struct conn_t {
   lambda_cost_t _old_lambda;
   int _old;
   /*
   _uchar conn;
    */
} conn_t;

static conn_t *st_brnchs;

long onode_t :: c_factor()
{
   long connection_factor = 0;
   city_id_t x;

   for (x=(city_id_t)(depth+1); x<sm->degree; x++)
      connection_factor += (mic_list[x].brnchs-2)*(mic_list[x].brnchs-2);
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
 *
 * Point *1*: if it is a tour make sure the optimal is the tours value.  this
 * helps when calculating the optimal is degraded by precision.
 */
int onode_t :: sort_lambda_costs(const sum_t optimost)
{
   city_id_t x;
   long connection_factor=c_factor();
   float TT;

   if (connection_factor == 0) { /*1*/
/* finds the length of a tour and assigns optimal
      city_id_t y, root;
      lambda_sum = 0;
      optimal = 0;
      for (x=0; x < sm->degree; x++)
         micmap[mic_list[x].span_id] = mic_list+x;
      root = mic_list[0].span_id;
      for (x = 1; x < (city_id_t)(depth+1); x++) {
         optimal += matrix->val(root, mic_list[x].span_id);
         root = mic_list[x].span_id;
      }
      x = root;
      for (y=one_tree_city; root=y, (y=micmap[y]->sindex) != NO_ID; x=root) {
         optimal += matrix->val(x, root);
      }
      optimal += matrix->val(x, root);
      for (y = initcost->id, x=0; y != root; x++, y=micmap[y]->sindex) {
         assert(x < sm->degree-depth-1);
         st_labels[x].label = y;
      }
      while (x>0) {
         optimal += matrix->val(root, st_labels[--x].label);
         root = st_labels[x].label;
      }
      optimal += matrix->val(root, mic_list[0].span_id);
*/
      return 1;
   }
   if (optimost<=optimal)
      return 2;

   TT = alpha*((float)(optimost - optimal))/connection_factor;

   for (x=(city_id_t)(depth+1); x<sm->degree; x++) {

      /*
      sm->lambdas[mic_list[x].span_id] += (lambda_t)
         (0.6*TT*(mic_list[x].brnchs-2) + 0.4*TT*st_brnchs[x]._old);
      st_brnchs[x]._old = (mic_list[x].brnchs-2);
       */

      st_brnchs[x]._old_lambda *= .378;
      st_brnchs[x]._old_lambda += .37*TT*((float)mic_list[x].brnchs-2);
      sm->lambdas[mic_list[x].span_id] += (lambda_t)st_brnchs[x]._old_lambda;
   }

   sm->sort(matrix, sm->lambdas);
   return 0;
}

/* Point *1*: The lambdas associated with the cities already traveled don't
 * affect lambda training, but do get lambda addition.
 *
 */
inline void onode_t :: sum_lambdas()
{
   city_id_t x;

   lambda_sum = (lambda_sum_t)0;
#ifdef LAMPATHCOST
/*bob*/
   for (x=0; x<sm->degree; x++)
      lambda_sum -= sm->lambdas[x];
   lambda_sum *= 2;
#else
   for (x=(city_id_t)(depth+1); x<sm->degree; x++)
      lambda_sum += sm->lambdas[mic_list[x].span_id]
         * ((lambda_sum_t)mic_list[x].brnchs-2);
#endif
   optimal += (sum_t)lambda_sum;
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
int onode_t :: train_lambdas(sum_t optimost, int max_iters)
{
   int count, iters, non_improvements=0, is_tour=0;
   onode_t *current_node;
   SortedMatrix sorted_matrix(*sm);

   alpha = START_ALPHA;
   alpha_factor = ALPHA_FACTOR(max_iters);
   st_brnchs = new conn_t[matrix->degree];
   for (count = 0; count < sm->degree; count++)
      st_brnchs[count]._old_lambda = 0.0;
      /*
      st_brnchs[count]._old = 0;
       */

   iters = 0;
   min_span();
   sum_lambdas();

   if (param.verbose>1 && GENERAL_MAX_ITERATIONS != max_iters) {
      dump.form("First 1-Tree = %d\n", (int)optimal);
      dump.flush();
   }
   current_node = new onode_t(*this, &sorted_matrix);

   check_connections();
   do {
      if ((is_tour=current_node->sort_lambda_costs(optimost)) != 0) {
         if (is_tour != 1)
            is_tour = 0;
         else {
            *this = *current_node;       /*1*/
         }
         break;
      }

      current_node->min_span();
      current_node->sum_lambdas();
      current_node->check_connections();

      if (param.verbose>2 && GENERAL_MAX_ITERATIONS != max_iters) {
         dump << "LAMBDA SUM " << current_node->lambda_sum << "\n";
      }

      if (current_node->optimal > optimal) {
         if (current_node->optimal > optimal + (sum_t)OPT_DIFF_THRESHOLD)
            non_improvements=0; 
         *this = *current_node;
         count = iters;
         if (param.verbose>1 && GENERAL_MAX_ITERATIONS != max_iters)
            dump.form("%7d Langrage one-tree: %f\n", (int)iters,(float)optimal);
      }
      else {
         if (non_improvements++ > (int)OPT_DIFF_BREAKS)
            break;
#ifdef DECREASE
         alpha *= alpha_factor;
         if (param.verbose>2 && GENERAL_MAX_ITERATIONS != max_iters)
            dump << "Alpha " << alpha << "\n";
#endif
      }
   } while (iters++<max_iters+1);

   check_connections();

   if (current_node->optimal > optimal) {
      *this = *current_node;
      count = iters;
      assert(0);
   }

   /*
   sm->sort(matrix, sm->lambdas);
    */

   if (param.verbose>1 && GENERAL_MAX_ITERATIONS != max_iters) {
      dump << "FINAL LAMBDA SUM " << lambda_sum << "\n";
      dump << "ITERS " << iters << " MAX " << max_iters << " highest_opt ->["
          << optimal << "]<- Used at iters " << count << "\n";
   }

   delete current_node;
   delete st_brnchs;
   check_connections();
   return is_tour;
}

inline void onode_t :: init_first_constructor(city_id_t init_id,
   SortedMatrix *m, TSPSolver *s)
{
   city_id_t x;
   mic_listT_t *ml;

   solver = (OTreeSolver*)s;
   sm = m;
   matrix = s->matrix;
   install_blocked_mallocs(m->degree*sizeof(mic_listT_t), 100);
   ml = mic_list = new mic_listT_t[m->degree];
   ml->span_id = init_id;
   for(x=0; x<m->degree; x++) {
      sm->lambdas[x] = (lambda_t)0;
      if (x != init_id) {
         ml++;
         ml->span_id = x;
      }
      ml->impeded = NOT_IMPEDED;
   }
   depth = 0;
   pure_depth = NO_ID;
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
onode_t :: onode_t (city_id_t init_id, SortedMatrix *m, TSPSolver *s,
   sum_t optimost)
{
   init_first_constructor(init_id, m, s);
   if (train_lambdas(optimost, INITIAL_MAX_ITERATIONS)) {
      pure_depth = depth;
      depth = (city_id_t)(sm->degree-2);
   }
   order(optimal, depth);
   assert(sm->lambdas != NULL);
}

/* Point *1*: tthere was a sort here before,
 * but it looks like i don'tneed it now that i'm using the matrix stack.
 * sm->sort(matrix);
 */
onode_t :: onode_t (city_id_t init_id, SortedMatrix *m, TSPSolver *s)
{
   init_first_constructor(init_id, m, s);
   /*1*/
   min_span();
   sum_lambdas();
   order(optimal, depth);
}

/* make_root ensures the mic_list who has span_id == id sindex to NO_ID and
 * will be the root of the tree it exists on after the function is called.
 * It does this by following its existing sindex pointer untill it finds
 * the current root, and along the way it reverses the sindex pointers
 * to point towards the id rather than towards the direction it is following.
 * If micmap[id]->sindex is already NO_ID then it is already the root of
 * its tree and nothing id done.
 *
 * Point *1*: we best be carefulling assigning sindexes so we save this as
 * new_sindex just until we find the new_id, using the micmap[old_id]->sindex.
 *
 * Point *1*: revisited we don't need to find the sindex since we know
 * which id its pointing to and now that sindexs are all id's whe just assign
 */
void onode_t :: make_root(city_id_t id)
{
   city_id_t new_id, old_id, new_sindex;

   if (micmap[id]->sindex != NO_ID) {
      old_id = id;
      new_id = micmap[old_id]->sindex;
      while (micmap[new_id]->sindex != NO_ID) {
         /*
         new_sindex = find_sindex(sm, micmap[new_id], old_id);
          */
         new_sindex = old_id; /*1*/
         old_id = new_id;
         new_id = micmap[old_id]->sindex;
         micmap[old_id]->sindex = new_sindex;
      }
      micmap[new_id]->sindex =  old_id;
      micmap[id]->sindex = NO_ID;
   }
}

/* this labels the st_labels list with which id belongs to which tree.
 * the "label" passed is the label that should be marked for ids
 * with out a tree.
 */
static void Label(city_id_t depth, city_id_t degree,
   mic_listT_t *mic_list, const city_id_t label)
{
   register city_id_t x,y,ny;

   for (x=0; x<degree; x++) {
      st_labels[x].label = label;
      st_labels[x].traveled = 0;
   }
   
   for (x=depth; x<degree; x++) {
      for (y=mic_list[x].span_id; (ny=micmap[y]->sindex) != NO_ID; ) {
         if ((y = st_labels[y].label) != label) {
            break;
         }
         y = ny;
      }
      st_labels[mic_list[x].span_id].label = y;
      st_labels[mic_list[x].span_id].traveled = 1;
   }
}

inline void onode_t :: do_impedance(sum_t most)
{
   city_id_t id;
   const lambda_t *lams=sm->lambdas;
   mic_listT_t *id_ml=NULL;
   cost_t *to_cost=matrix->cost[mic_list[depth].span_id];
   sum_t ruk=most;

#ifdef LAMPATHCOST
/*bob*/
   ruk -= (sum_t)lams[mic_list[depth].span_id] - mic_list[depth].pathcost;
#else
   ruk += (sum_t)lams[one_tree_city] + mic_list[depth].pathcost;
#endif
   for (id = (city_id_t)(depth+1); id<matrix->degree; id++) {
      if (optimal
       + to_cost[mic_list[id].span_id]
#ifdef LAMPATHCOST
       + (sum_t)lams[mic_list[id].span_id]
#else
       + (sum_t)lams[mic_list[id].span_id]
#endif
       >= ruk)
         mic_list[id].impeded = IMPEDED;
      else {
         id_ml = mic_list+id;
         id_ml->impeded = NOT_IMPEDED;
      }
   }
   if (id_ml == NULL)
      optimal = MAX_SUM;
   else
      id_ml->impeded = LAST_NOT_IMPEDED;
}

/* Point *10*: this is where we clean up some branching problems.  We
 * don't take out the old_one_tree city if the old_one_tree_city is
 * the chosen sub city.  We also don't take out the last_sindex branch
 * if there was no last_sindex.  Also the last_sindex will never be the
 * chosen sub since the sindex is off of the chosen sub.
 */
inline void onode_t :: incremental_min_span(city_id_t id,
   const city_id_t last_sindex)
{
   mic_listT_t *ml=mic_list+depth;
   static city_id_t trees[MAX_SPAN_CONNECTIONS];
   city_id_t the_tree = NO_ID, num_trees;
   city_id_t a_sindex, newtree, last_id = NO_ID;
   city_id_t old_one_tree_city = one_tree_city;
   SortedCost *base, *low_base=NULL;
   cost_t base_cost, lambda_base_cost;
   lambda_cost_t lowest_cost;

   if (last_sindex != NO_ID) {
      /*7*/
#ifdef LAMPATHCOST
      optimal -= lowest_cost = ml->pathcost =
         (cost_t)((sum_t)matrix->val(id,last_sindex)
         + sm->lambdas[id] + sm->lambdas[last_sindex]);
#else
      optimal -= ml->pathcost = matrix->val(id,last_sindex);
      lowest_cost = (cost_t)((sum_t)ml->pathcost
       + sm->lambdas[id] + sm->lambdas[last_sindex]);
#endif
      lambda_sum -= (lambda_sum_t)sm->lambdas[last_sindex] + sm->lambdas[id];
      one_tree_city = last_sindex;
      num_trees=1;
   }
   else {
      lowest_cost = MAX_LCOST;
      ml->pathcost = MAX_COST;
      one_tree_city = NO_ID;
      num_trees=0;
   }

   for (id = (city_id_t)(depth+1); id < sm->degree; id++) {   /*1*/
      micmap[mic_list[id].span_id] = mic_list+id;
      if (mic_list[id].sindex == NO_ID)        /*9*/
         continue;
      if (mic_list[id].sindex == ml->span_id) {
#ifdef LAMPATHCOST
         base_cost = lambda_base_cost =
            (cost_t)((sum_t)matrix->val(mic_list[id].span_id, ml->span_id)
            + sm->lambdas[mic_list[id].span_id]
            + sm->lambdas[ml->span_id]);
#else
         base_cost = matrix->val(mic_list[id].span_id, ml->span_id);
         lambda_base_cost = (cost_t)((sum_t)base_cost 
            + sm->lambdas[mic_list[id].span_id]
            + sm->lambdas[ml->span_id]);
#endif
         optimal -= base_cost;
         lambda_sum -=
            sm->lambdas[mic_list[id].span_id]
            + sm->lambdas[ml->span_id];
         mic_list[id].brnchs--;
         if (lowest_cost > lambda_base_cost) {
            lowest_cost = lambda_base_cost;
            ml->pathcost = base_cost;
            one_tree_city = mic_list[id].span_id;
         }
         mic_list[id].sindex = NO_ID;
         assert(num_trees < MAX_SPAN_CONNECTIONS);
         trees[num_trees++] = the_tree = mic_list[id].span_id;
      }
   }
   if (last_sindex != NO_ID)
      micmap[last_sindex]->brnchs--;
   if (old_one_tree_city != ml->span_id)    /*10*/
      micmap[old_one_tree_city]->brnchs--;

   assert(ml->pathcost != MAX_COST);
   optimal += ml->pathcost;
   lambda_sum += (lambda_sum_t)sm->lambdas[one_tree_city] + sm->lambdas[ml->span_id];
   micmap[one_tree_city]->brnchs++;
   Label((city_id_t)(depth+1), sm->degree, mic_list, the_tree);
   if (initcost->id == ml->span_id) {    /*6*/
#ifdef LAMPATHCOST
      optimal -= initcost->cost;
#else
      optimal -= initcost->real_cost;
#endif
      lambda_sum -= sm->lambdas[ml->span_id];
      do {
         initcost++;
      } while (!st_labels[initcost->id].traveled);
      assert(initcost->cost != MAX_LCOST);
#ifdef LAMPATHCOST
      optimal += initcost->cost;
#else
      optimal += initcost->real_cost;
#endif
      lambda_sum += sm->lambdas[initcost->id];
      micmap[initcost->id]->brnchs++;
   }
   lowest_cost = MAX_LCOST;
   for (num_trees--; num_trees > 0; num_trees--) { /*2*/
      if (lowest_cost != MAX_LCOST) {
         newtree = st_labels[mic_list[last_id].sindex].label; /*5*/
         for (id = 0; id < sm->degree; id++)
            if (st_labels[id].label == newtree)
               st_labels[id].label = the_tree;
      }
      lowest_cost = MAX_LCOST;
      for (id = (city_id_t)(depth+1); id < sm->degree; id++) {
         if (st_labels[mic_list[id].span_id].label == the_tree) {
            base = sm->cost[mic_list[id].span_id];
            for (a_sindex=0;; a_sindex++) { /*11*/
               assert(a_sindex < sm->degree);
               if (st_labels[base[a_sindex].id].label != the_tree) {
                  if (base[a_sindex].cost < lowest_cost) {
                     lowest_cost = base[a_sindex].cost;
                     low_base = base+a_sindex;
                     last_id = id;
                  }
                  break;
               }
            }
         }
      }
      make_root(mic_list[last_id].span_id); /*4*/
      mic_list[last_id].sindex = low_base->id; /*3*/
#ifdef LAMPATHCOST
      optimal += low_base->cost;
#else
      optimal += low_base->real_cost;
#endif
      lambda_sum += sm->lambdas[mic_list[last_id].span_id]
         + sm->lambdas[low_base->id];
      mic_list[last_id].brnchs++; micmap[low_base->id]->brnchs++;
   }

#ifndef LAMPATHCOST
   optimal += (sum_t)lambda_sum;
#endif

}

/* onode_t :: spawn()
 *
 * here we solve the N minus one tree.  This consists of assuming you
 * have a one-tree of degree (degree-depth) and you need to travel the
 * id (sub), essentially removing that id from the one tree sticking
 * it on the subproblem's path, and then solving the smaller one-tree of
 * (degree-depth-1).
 *
 * WITH OUT CYCLE CHECKS
 *
 * This is done by first discriminating the id's of the "cut" one tree
 * to see which tree each id is in and labeling that id to the tree it
 * is in.  When you "cut" a minimum spanning tree, you essentially break
 * it into one or more tree's, the number determined by how many edges
 * connected the id you take out with in the min spanning tree.
 *
 * Note nothing needs to be done if the Id being cut from the min spanning
 * tree, only has one edge in the min spanning tree, since you have a min
 * spanning tree then.
 *
 * If there are more than one trees after the "cut", you then must pick
 * a tree and find the shortest edge from that tree to the rest of the
 * graph by looking down the sorted costs and finding the first cost that
 * does not go to a tree that is labeled.
 *
 * Once you find this edge, you then "connect" the tree to the edge that
 * the edge travels to making a new tree.  You then find the shortest
 * edge from the new tree to any other id in the graph.  This is done
 * iteratively untill you only have two trees left which when you find
 * the shortest edge between them, you have made a min spanning tree.
 *
 * Point *1*: find the edges on the min span tree that were connected to
 * the id being traveled and cut those edges out of the tree.  This is
 * also the same as finding the roots of the trees separated by the cutting.
 *
 * Point *2*: We loop through untill num_trees is greater than zero because
 * we don't have to do the last tree.
 *
 * Point *3*: make a connection between trees.  If the sindex of the vertex
 * we are at doesn't point anywhere then we can easily make the connection.
 *
 * Point *5*: here we label the new tree as the old trees label id, to complete
 * the connection of trees.  The new treee is the sorted cost id of the last
 * id of the old tree.
 *
 * Point *6*: if the shortest cost from the initial id is going to the
 * subproblem's id, then it needs to find another.
 *
 * Point *7*: setup the fininding of the lowest cost from the sub node.
 * ml->pathcost will be this value (i.e. one of the two special edges of the
 * one tree.)
 *
 * Point *8*: Don't even let the node constructed get added on to the queue
 * if the node_t constructed does not have any children of its own.
 *
 * Point *9*: if there are no edges from the node (it is a root) then there
 * is no way to check if the edge is connected to the id being traveled.
 *
 * Point *11*: there is no security to trying to look for a sindex that is
 * always greater than the sindex the min span edge points to, such as :
 * a_sindex = (mic_list[id].sindex==NO_ID) ? 0 : mic_list[id].sindex;
 * So we have to start with a_sindex==0 and search through there.  we can
 * later just have a linked list so we don't have to iterate, but for now
 * if we don't start at zero the a_sindex we could be looking for is anywhere
 * the lowest, not just the lowest after the min span edge.
 *
 * Point *12*: if val is used then we must include lambdas for a langrangian
 * cH Ad e H
 */
inline void onode_t :: spawn(const onode_t &parent, const city_id_t sub,
   const sum_t most)
{
   mic_listT_t *ml, *pml;
   mic_listT_t *id_ml = parent.mic_list+sub;
   city_id_t id = id_ml->span_id, last_sindex = id_ml->sindex;

   if (param.verbose>10)
      print();
   if (param.verbose>2)
      dump << "{" << (most - optimal) << "}\n";

#ifdef PRINT_IT
if (param.verbose>1)
print();
#endif
   ml = mic_list + (++depth);
   pml = mic_list + (city_id_t)(id_ml-parent.mic_list);
#ifndef LAMPATHCOST
   optimal -= (sum_t)lambda_sum;
#endif
   optimal -= (ml-1)->pathcost;
   lambda_sum -= sm->lambdas[one_tree_city];
   *pml = *ml;
   ml->span_id = id;
#ifdef LAMPATHCOST
/*bob*/
   (ml-1)->pathcost = (cost_t)((sum_t)matrix->val((ml-1)->span_id, id)
         + sm->lambdas[(ml-1)->span_id] + sm->lambdas[id]);
#else
   (ml-1)->pathcost = matrix->val((ml-1)->span_id, id);
#endif
   optimal += (ml-1)->pathcost;
   lambda_sum += sm->lambdas[id];

      incremental_min_span(id, last_sindex);

   /*
   onode_t test(*this);
   test.min_span();
   test.sum_lambdas();
   if (fabs(optimal - test.optimal) > .0001) {
      dump << "   " << optimal << " <> " << test.optimal << "\n";
      test.print();
   }
    */

#ifdef PRINT_IT
if (param.verbose>1) {
   dump << "\n";
   print();
}
#endif
   check_connections();
   do_impedance(most);
   order(optimal, depth);
}

void onode_t :: construct(const onode_t &parent)
{
   solver = parent.solver;
   matrix = parent.matrix;
   sm = parent.sm;
   mic_list = new mic_listT_t[matrix->degree];
   *this = parent;
}

onode_t :: onode_t (const onode_t &parent, const city_id_t sub, const sum_t most)
{
   construct(parent);
   spawn(parent, sub, most);
}

onode_t * onode_t :: rebirth(const city_id_t sub, const sum_t most)
{
   spawn(*this, sub, most);

   return this;
}

onode_t :: onode_t (const onode_t &parent)
{
   construct(parent);
}

onode_t :: onode_t (const onode_t &parent, SortedMatrix *new_sm)
{
   solver = parent.solver;
   matrix = parent.matrix;
   sm = new_sm;
   mic_list = new mic_listT_t[matrix->degree];
   *this = parent;
}

otree_stats st_stats;

void init_otree_stats()
{
   short x;
   st_stats.Trains = st_stats.Explores = 0;
   for (x=0; x<MAX_OT_STATS; x++) {
      st_stats.dist[x] = 0;
   }
}

void free_otree_stats(Term &term)
{
   short x;
   term << "Ostats : ";
   term << "Trained " << ((st_stats.Explores == 0) ? 0 :
      st_stats.Trains*100/st_stats.Explores) << "% of "
      << st_stats.Explores << " Explores\n";
   for (x=0; x<MAX_OT_STATS; x++) {
      if (st_stats.dist[x] != 0) {
         term << (unsigned short)x << "%==" << st_stats.dist[x] << ", ";
      }
   }
   term << "\n";
}

void onode_t :: branch(PQueue &que)
{
   SortedMatrix *savesm = sm;
   city_id_t x;
   onode_t *node;

   if (stat_will_train(que.most)) {
      make_lone_matrix(&sm);
      savesm = sm;
      if (train_lambdas(que.most, GENERAL_MAX_ITERATIONS)) {
         // impede everyone because this is a tour!
         pure_depth = depth;
         if (param.verbose>2)
            dump << "!!!!!!!!!TOUR FOUND at level " << depth << "\n";
         depth = (city_id_t)(sm->degree-2);
         order(optimal, depth);
         if (!que.enq(this))
            unuse_matrix(savesm);
         que.dont_delete_last_dequeued();
         return;
      }
      st_stats.Trains++;
   }
   st_stats.Explores++;
   for (x=(city_id_t)(depth+1); x<savesm->degree; x++) {
      switch (mic_list[x].impeded & IMP_MASK) {
      case NOT_IMPEDED:
         use_matrix(sm);
         if (!que.enq( node = new onode_t (*this, x, que.most) ))
            unuse_matrix(sm = savesm);
         else
            node->check_connections();
         break;
      case LAST_NOT_IMPEDED:
         use_matrix(sm);
         if (!que.enq( node = rebirth(x, que.most) ))
            unuse_matrix(savesm);
         else
            node->check_connections();
         que.dont_delete_last_dequeued();
         x = savesm->degree; /*2*/
         break;
      }
   }
   unuse_matrix(savesm);
}
