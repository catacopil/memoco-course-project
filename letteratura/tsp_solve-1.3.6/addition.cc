/* addition.cc
 *
 * KNOWN BUGS: will fail if entered with a tsp with identical pos_t's.
 *
 * nearest addition and fathest insertion, with initial convex hull and
 * maximal angle selections for euclidean.
 *
 * orig ceh 12-91
 *
 * c++ conversion ceh 12-93
 * added farthest insertion functionality 9-94
#define PRINTIT
 */

#define MAX_SUM_COST MAX_COST
/*
#define MAX_SUM_COST MAX_SUM
 */

#include "addition.h"
#include <assert.h>

inline void AdditionHeuristic :: internal_add(const city_id_t to,
   city_id_t &from)
{
   city_id_t i;

   assert(to != NO_ID);
   if (from == NO_ID) {
      if (cities_traveled == degree)
         i = last_traveled = to;
      else
         i = traveled[last_traveled];
      from = last_traveled;
   }
   else
      i = traveled[from];
#ifdef PRINTIT
dump.form("%6d put %d<-%d<-%d\n", (int)(matrix->val(to,traveled[from])
+matrix->val(from,to)-matrix->val(from,traveled[from])),
(int)traveled[from],(int)to,(int)from);
#endif
   last_traveled = traveled[from] = to;
   traveled[to] = i;
}

inline double AdditionHeuristic :: dot_product(const pos_t *p1, const pos_t *p2,
   const pos_t *p3)
{
   return (p1->x-p2->x)*(p3->x-p2->x) + (p1->y-p2->y)*(p3->y-p2->y);
}

inline void AdditionHeuristic :: farthest_internal_select(city_id_t &to,
   city_id_t &from)
{
   sum_t the_lowest, save_cost;
   city_id_t *t, j;
   cost_t *outcost;
/*
pos_t *p1;
 */

   the_lowest = COST_MAX;
   to = farthestcity;
/*
p1 = matrix->pos+to;
 */
   for (t = traveled; t<traveled_end; t++) { /*2*/
      if (*t == NO_ID)
         continue;
      outcost = matrix->cost[(city_id_t)(t-traveled)];
      j = *t;
/*
save_cost = dot_product(matrix->pos+(t-traveled), p1, matrix->pos+j)
   / (outcost[to] * matrix->val(to,j));
    */
      save_cost = matrix->val(to,j) + outcost[to] - outcost[j];
      if (save_cost < the_lowest) {
         the_lowest = save_cost;
         from = (city_id_t)(t-traveled);
      }
   }
   assert(the_lowest != COST_MAX);
}

// travel "to_travel" after city "from" in the tour so far
void AdditionHeuristic :: farthest_add(city_id_t to_travel)
{
   cost_t farthestcost;
   cost_t *fcost = nearcost, *outcost;
   city_id_t *t, from = NO_ID;

   if (to_travel == NO_ID)
      farthest_internal_select(to_travel, from);
   internal_add(to_travel, from);
   if (--cities_traveled == 0)
      return;
   farthestcost = MIN_COST;
   farthestcity = NO_ID;
   outcost = matrix->cost[to_travel];
   for (t = traveled; t<traveled_end; t++, fcost++) {
      if (*t != NO_ID)
         continue;
      if (*fcost > outcost[(city_id_t)(t-traveled)])
         *fcost = outcost[(city_id_t)(t-traveled)];
      if (farthestcost < *fcost) {
         farthestcost = *fcost;
         farthestcity = (city_id_t)(t-traveled);
      }
   }
   assert(nearestcity != NO_ID);
   assert(farthestcity != NO_ID);
}

inline void AdditionHeuristic :: addition_internal_select(city_id_t &to,
   city_id_t &from)
{
   to = nearestcity;
   from = farthestcity;
}

void AdditionHeuristic :: addition_add(city_id_t to_travel)
{
   cost_t *ncost = nearcost, nearestcost;
   cost_t back_cost, back_cost2, *outcost, *jcost, *outcost2, necost;
   city_id_t *t, from = NO_ID, to_to, j, *fcity = farcity;
/*
pos_t *p1, *p2, *p3, *p4;
 */

   if (to_travel == NO_ID)
      addition_internal_select(to_travel, from);
   internal_add(to_travel, from);
   if (--cities_traveled == 0)
      return;
   assert(from != NO_ID);
   nearestcost = MAX_SUM_COST;
   nearestcity = NO_ID;
   to_to = traveled[to_travel];
   outcost = matrix->cost[to_travel];
   outcost2 = matrix->cost[from];
/*
p1 = matrix->pos+from;
p3 = matrix->pos+to_travel;
p4 = matrix->pos+to_to;
 */
   if (to_travel == to_to)
      back_cost = 0;
   else
      back_cost = outcost[to_to];
   if (to_travel == from)
      back_cost2 = 0;
   else
      back_cost2 = outcost2[to_travel];
   for (t = traveled; t<traveled_end; t++, ncost++, fcity++) {
      if (*t != NO_ID)
         continue;
      j = (city_id_t)(t-traveled);
/*
p2 = matrix->pos+j;
 */
      jcost = matrix->cost[j];
      if (*fcity == from) {
         city_id_t *tt;
         for (*ncost = MAX_SUM_COST, tt = traveled; tt<traveled_end; tt++) {
            if (*tt == NO_ID)
               continue;
            outcost = matrix->cost[(city_id_t)(tt-traveled)];
/*
p1 = matrix->pos+(tt-traveled);
p3 = matrix->pos+*tt;
necost = dot_product(p1, p2, p3)/(outcost[j] * jcost[*tt]);
 */
necost = outcost[j] + jcost[*tt] - outcost[*tt];
            if (*ncost > necost) {
               *ncost = necost;
               *fcity = (city_id_t)(tt-traveled);
            }
         }
         outcost = matrix->cost[to_travel];
/*
p1 = matrix->pos+from;
p3 = matrix->pos+to_travel;
 */
      }
      else {
/*
necost = dot_product(p3, p2, p4) / (outcost[j]*jcost[to_to]);
 */
necost = outcost[j] + jcost[to_to] - back_cost;
         if (*ncost > necost) {
            *ncost = necost;
            *fcity = to_travel;
         }
/*
necost = dot_product(p1, p2, p3) / (outcost2[j]*jcost[to_travel]);
 */
necost = outcost2[j] + jcost[to_travel] - back_cost2;
         if (*ncost > necost) {
            *ncost = necost;
            *fcity = from;
         }
      }
      if (nearestcost > *ncost) {
         nearestcost = *ncost;
         nearestcity = j;
         farthestcity = *fcity;
      }
   }
   assert(nearestcity != NO_ID);
}

/* Point *1*: find nearest (farthest) cities from tour cities.
 *
 * Point *2*: find best insertion of nearest (farthest) city
 */
int AdditionHeuristic :: run()
{
   city_id_t k, l;

   assert(cities_traveled != degree);
   if (type == SPLIT_ADDITION) {
      city_id_t *t, *tt, *fcity = farcity, j;
      sum_t necost, nearestcost;
      cost_t *ncost = nearcost, *outcost, *jcost;

      while (cities_traveled > 3*degree/4)
         (this->*to_add)(NO_ID);
      nearestcost = MAX_SUM_COST;
      for (t = traveled; t<traveled_end; t++, ncost++, fcity++) {
         if (*t != NO_ID)
            continue;
         j = (city_id_t)(t-traveled);
         jcost = matrix->cost[j];
         for (*ncost = MAX_SUM_COST, tt = traveled; tt<traveled_end; tt++) {
            if (*tt == NO_ID)
               continue;
            outcost = matrix->cost[(city_id_t)(tt-traveled)];
            necost = outcost[j] + jcost[*tt] - outcost[*tt];
            if (*ncost > necost) {
               *fcity = (city_id_t)(tt-traveled);
               if (nearestcost > (*ncost = necost)) {
                  nearestcost = necost;
                  nearestcity = j;
                  farthestcity = *fcity;
               }
            }
         }
      }
      to_add = &AdditionHeuristic::addition_add;
   }
   while (cities_traveled)
      (this->*to_add)(NO_ID);
   for (k = l = 0; l<degree; l++)
      tour->travel(k = traveled[k]);
   return 0;
}

/* Define a struct to hold positions and sites to be sorted by qsort
 */
class poses_t {
public:
   inline poses_t() {};
   pos_t pos;
   city_id_t site;
};

/* compare_poses
 *
 * the compare function to pass to qsort to sort the list of all edges
 */
static int compare_poses(const void *p1, const void *p2)
{
   double t = (((poses_t *)p1)->pos.x - ((poses_t *)p2)->pos.x);

   if (t > 0.0)
      return 1;
   if (t < 0.0)
      return -1;
   return 0;
}

/* LINE_T
 *
 * a line structure to be a part of a convex hull, with slope and
 * y-intercept.
 */
class line_t {
public:
   inline line_t() {};
   double slope;
   double y_intercept;
   poses_t *last_pose;
   city_id_t site;
};

#define MAX_LINES 100

/* HULL_T
 *
 * a convex hull structure, 4 of which are created to drape around the
 * set of points at the four corners..
 */
class hull_t {
public:
   inline hull_t() {};
   line_t lines[MAX_LINES];
   short line;
   poses_t *last_pose;

   void add_line_to_hull(poses_t *pose);
   void climb_under_hull(poses_t *end_pose, int inc);
   void climb_over_hull(poses_t *end_pose, int inc);
};

/* add_line_to_hull
 *
 * simply add a line to the end of the hull, that will connect the new point
 * to the last point in the hull
 */
void hull_t :: add_line_to_hull(poses_t *pose)
{
   line_t *h_line = lines+line;
   double save;

   h_line->site = pose->site;

   if ((save = pose->pos.x-last_pose->pos.x) == 0.0) {
      h_line->slope = FLOAT_MAX;
      h_line->y_intercept = FLOAT_MAX;
   }
   else {
      h_line->slope = (pose->pos.y-last_pose->pos.y) / save;
      h_line->y_intercept = pose->pos.y - (h_line->slope * pose->pos.x);
   }

   lines[line++].last_pose = last_pose;

   assert (line <= MAX_LINES);
   last_pose = pose;
}

/* Macros to determine if a line is higher/lower than a point
 *
 * Note the '>' and '<' are warped because pixels are upside down from
 * a mathematical graph...
 */
#define line_lower(line,point)               \
   ((point)->pos.y >                         \
    (((line)->slope)*((point)->pos.x)) + (line)->y_intercept)
#define line_higher(line,point)              \
   ((point)->pos.y <                         \
    (((line)->slope)*((point)->pos.x)) + (line)->y_intercept)


/* climb_hull()
 *
 * here we try to all lines to hull until we reach the maximum point.
 *
 *                                                    end_pose
 * we are given a hull with a single line in it that         *_2_*
 * represents the lower edge, we then insert convex               \1
 * lines until we reach the upper edge.....                    pose*
 *                                                                  \0
 * climb_over_hull() will climb over the edges assumnig you          *
 * start from a side and want to convex up.  climb_under_hull()
 * will climb under, if you wish to cover the underside of points
 * once started for the side most point.
 */
void hull_t :: climb_over_hull(poses_t *end_pose, int inc)
{
   short here_line = (short)(line-1);
   poses_t *pose = last_pose, *last_pose = pose;

   /* HI stuff ONLY */
   while (pose != end_pose) {
      pose += inc;
      if (pose->pos.y < last_pose->pos.y) /* if point is lower than last line */
         continue;                        /* then the point isn't worth it */
      /* find a line in hull which will enable a convex line
       */
      if (here_line >= 0) {
         while (line_lower(lines+here_line, pose)) {
            if (--here_line<0)
               break;
         }
      }
      if (line != ++here_line) {
         last_pose = lines[here_line].last_pose;
         line = here_line;
      }
      add_line_to_hull(pose);
      last_pose = pose;
   }
}

/* the only difference in the function below (climb_under_hull)() and the
 * function above is the line where "line_lower" is replaced by "line_higher"
 *
 */

void hull_t :: climb_under_hull(poses_t *end_pose, int inc)
{
   short here_line = (short)(line-1);
   poses_t *pose = last_pose, *last_pose = pose;

   /* HI stuff ONLY */
   while (pose != end_pose) {
      pose += inc;
      if (pose->pos.y > last_pose->pos.y) /* if point is lower than last line */
         continue;                        /* then the point isn't worth it */
      /* find a line in hull which will enable a convex line
       */
      if (here_line >= 0) {
         while (line_higher(lines+here_line, pose)) {
            if (--here_line<0)
               break;
         }
      }
      if (line != ++here_line) {
         last_pose = lines[here_line].last_pose;
         line = here_line;
      }
      add_line_to_hull(pose);
      last_pose = pose;
   }
}

int AdditionHeuristic :: can_run(const Matrix *) const
{
   return 1;
}

AdditionHeuristic :: ~AdditionHeuristic()
{
   delete traveled;
   delete nearcost;
}

#define HILEFT (Hulls[0])
#define LOLEFT (Hulls[1])
#define HIRIGHT (Hulls[2])
#define LORIGHT (Hulls[3])
// hull_t HILEFT, LOLEFT, HIRIGHT, LORIGHT;
AdditionHeuristic :: AdditionHeuristic(const Matrix *m, int ty) : TourFinder(m)
{
   cost_t *ncost;
   city_id_t x, x_high, x_low, *t;
   poses_t *y_highest = NULL, *y_lowest = NULL, *poses;
   double highest_y = 0., lowest_y = COST_MAX;
   double save;
   short line_right;

   switch (ty&TYPEMASK_ADDITION) {
      case SPLIT_ADDITION: case FARTHEST_ADDITION:
         to_add = &AdditionHeuristic::farthest_add;
         break;
      case ANGLE_ADDITION:
         /*
         if (m->is_geometric_2d())
            to_add = angle_add;
         else
            to_add = addition_add;
         break;
          */
      default:
      case NORMAL_ADDITION: to_add = &AdditionHeuristic::addition_add; break;
   }
   type = ty;
   ncost = nearcost = new cost_t[degree];
   traveled = new city_id_t[degree*2];
   farcity = traveled+degree;
   traveled_end = traveled+degree;
   for (t = traveled; t<traveled_end; t++, ncost++) {
      *t = NO_ID;
      *ncost = MAX_SUM_COST;
   }
   cities_traveled = degree;
   farthestcity = NO_ID;
   if (!m->is_geometric_2d() || degree < 4) {
      city_id_t init;
      init = param.initial_choice;
      if (init == NO_ID)
         init = 0;
      (this->*to_add)(init);
      return;
   }
   poses = new poses_t[degree];
   for (x = 0; x<degree; x++) {
      poses[x].site = x;
      poses[x].pos = matrix->pos[x];
   }
   qsort(poses, (size_t)degree, (size_t)sizeof(poses_t), compare_poses);

   for (x = 0; x<degree; x++) {
      save = poses[x].pos.y;
      if (save > highest_y) {
         highest_y = save;
         y_highest = poses+x;
      }
      if (save < lowest_y) {
         lowest_y = save;
         y_lowest = poses+x;
      }
#ifdef PRINTIT
dump.form("%d at (%lf,%lf)   %lx\n", (int)poses[x].site, poses[x].pos.x,
save, (long)(poses+x));
#endif
   }
   assert(y_lowest != NULL && y_highest != NULL);

   /* Initialize the four hulls that will drape the points...
    * HIRIGHT is the upper right corner, LOLEFT is the lower left corner
    * the LORIGHT is in mathematical quadrant I.
    */
   hull_t *Hulls = new hull_t[4];
   HIRIGHT.line = HILEFT.line = LORIGHT.line = LOLEFT.line = 0;
   HIRIGHT.last_pose = poses+(degree-1);
   LORIGHT.last_pose = poses+(degree-1);
   HILEFT.last_pose = poses;
   LOLEFT.last_pose = poses;
   x_high = (poses+(degree-1))->site;

   x_low  = (poses)->site;

   HIRIGHT.climb_over_hull(y_highest, -1);
   HILEFT.climb_over_hull(y_highest, 1);
   LORIGHT.climb_under_hull(y_lowest, -1);
   LOLEFT.climb_under_hull(y_lowest, 1);

   delete poses;

   line_right = 0;
   if (HIRIGHT.line>0) {           /* Only do if there is >1 line(s) */
      (this->*to_add)(x_high);
      while (line_right < HIRIGHT.line-1) {
         (this->*to_add)(HIRIGHT.lines[line_right].site);
         line_right++;
      }
   }

   line_right = (short)(HILEFT.line-1);
   while (line_right >= 0) {
      (this->*to_add)(HILEFT.lines[line_right].site);
      line_right--;
   }

   line_right = 0;
   if (LOLEFT.line>0) {            /* Only do if there is >1 line(s) */
      (this->*to_add)(x_low);
      while (line_right < LOLEFT.line-1) {
         (this->*to_add)(LOLEFT.lines[line_right].site);
         line_right++;
      }
   }

   line_right = (short)(LORIGHT.line-1);
   while (line_right >= 0) {
      (this->*to_add)(LORIGHT.lines[line_right].site);
      line_right--;
   }
   delete Hulls;
}
