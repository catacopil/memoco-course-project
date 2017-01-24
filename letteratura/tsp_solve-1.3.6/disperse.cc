/* disperse.cc
 *
 * Dispersion Heuristic
 *
 * an iterative algorithm, which takes an Relaxed TSP solution, looks at the
 * "multi-from" nodes, and attempts to disperse all but one of the multi-from
 * paths to other travelable site, on the definition of the TSP tour which
 * each node must have an single entrance _and_an_single_ exit.
 *
 * Orig Nov, 1991 Chad Hurwitz
 *  modified 4-22-94 ceh converted to c++ tourfinder
 */

#include "disperse.h"
#include <assert.h>
#include "chnew.h"
#include "findtour.h"

/* this can't be 30 until i have the arborescence solver going, since sometimes
 * the asymmetric problems made are almost symmetric and the AP can't solve
 * them quickly.
 */
#define MTL 20
#define MAX_SYM_THINGS_LEFT  MTL /* the things left that you should just go */
                                /* when you are at a state to solve the */
                                /* best solution to the smaller set */
#define MAX_ASYM_THINGS_LEFT MTL
#define MAX_THINGS_LEFT max(MAX_ASYM_THINGS_LEFT,MAX_SYM_THINGS_LEFT)

/*
#define DPRINT
#define SHARKIE
 */

#ifdef DPRINT

static void print_part(part_t *var, int a)
{
   id_list_t *ivar;

   dump.form("(%d)  <-(%d)-{%-3d}  <-(%d)-{%-3d} ",
      (int)var->tail->id,
      (int) (var->orig_1rst == NULL || a==1? -1 : var->orig_1rst->head->id),
      (int)var->cost_1rst,
      (int) (var->orig_2nd == NULL || a==1? -1 : var->orig_2nd->head->id),
      (int) (var->cost_2nd < COST_MAX ? var->cost_2nd : -1));
   for (ivar = var->head; ivar != NULL; ivar = ivar->next)
      dump << ivar->id << ", ";
   dump.form("Trv:%d 2nd:%d\n", (int)var->traveled, (int)var->used_2nd);
}

static void print_parts(part_t *the_parts)
{
   part_t *var;

   for (var = the_parts; var != NULL; var = var->next) {
      print_part(var, 0);
   }
   dump.flush();
}

static void print_toids(part_t *parts, multi_from_t *the_froms)
{
   part_t *var;
   decision_site_t *dec_var;
   int x,y;

   for (var = parts; var != NULL; var = var->next) {
      x = var->head->id;
      assert(x < degree);
      dec_var = the_froms[x].decisions;
      for (y=0; y < the_froms[x].number_of_froms; y++) {
         dump.form(" %d,D%d (%d)  ->(%d)-{%-3d}    2nd_used %d\n",
          x, y, (int)(*dec_var->used_2nd == 1
            ? (dec_var->part->orig_2nd == NULL ? -1
               : dec_var->part->orig_2nd->head->id
               )
            : (dec_var->part->orig_1rst == NULL ? -1
               : dec_var->part->orig_1rst->head->id
               )
            ),
          (int)dec_var->part->tail->id, (int)(*dec_var->used_2nd == 1
            ? dec_var->part->cost_2nd
            : dec_var->part->cost_1rst),
          (int)*dec_var->used_2nd
         );
         dec_var = dec_var->next;
      }
   }
   dump.flush();
}

#endif

#ifdef DUMB

static void print_cycle_choice(cycle_choice_t *c)
{
   if (c->j_part == NULL)
      dump << "NULL CYCLE CHOICE\n";
   else {
   dump.form("CYCLE CHOICE (%d)j<-i(%d) JUDGE [%d]\n",
      (int)c->j_part->tail->id, (int)c->i_part->head->id,
      (int)c->judgement);
   }
   dump.flush();
}

#endif


/* update_cycle_choice
 *
 * the routine is called on every edge in a cycle, and if this cycle has
 * a higher judgement than the previous edge cycle choice "c", then the cycle
 * choice is changed to be this edge.
 *
 *
 * a future enhancment could be to take both lowest_xi and lowest_yj at the
 * same time only when there is one other part in "parts", and if there is more
 * than one part in "parts" then restrict x and y to be selected from unique
 * parts (i.e., you cant have a lowest_xi and a lowest_yj coming from the same
 * part.)
 */
void DispersionHeuristic :: update_cycle_choice(cycle_choice_t *c, part_t *j,
   part_t *i, part_t *parts)
{
   part_t *var=parts;
   cost_t save_cost;
// cost_t *jincost=map->sites[j->tail->id].in_cost;
   city_id_t jid = j->tail->id;
   cost_t *ioutcost=matrix->cost[i->head->id];
   cost_t lowest_xj=MAX_COST, lowest_iy=MAX_COST;
   cost_t lowest2_xj=MAX_COST, lowest2_iy=MAX_COST;
   sum_t judge;

#ifndef NDEBUG
if (matrix->cost[i->head->id][j->tail->id] != j->cost_1rst)
   dump << "SOMETHING IS TERRIBLY WRONG============================\n";
#endif
   
   while (var != NULL) {
      /* find j's 2ndMIC from other part since j's 1rstMIC is coming from i */
      assert(var->head->id < degree && jid < degree);
      save_cost = matrix->val(var->head->id, jid);
#ifdef DPRINT
if (param.verbose)
dump << "______POSIBLE X " << save_cost;
#endif
      if (save_cost < lowest_xj) {
         lowest2_xj = lowest_xj;
         lowest_xj = save_cost;
         j->orig_2nd = var;         /* temporarily store var */
      }
      else if (save_cost < lowest2_xj) {
         lowest2_xj = save_cost;
      }
      /* find the best out cost from i */
assert(var->tail->id < degree);
      save_cost = ioutcost[var->tail->id];
      assert(var->tail->id < degree);
#ifdef DPRINT
if (param.verbose)
dump << "______POSIBLE Y " << save_cost << "\n";
#endif
      if (save_cost < lowest_iy) {
         lowest2_iy = lowest_iy;
         lowest_iy = save_cost;
         i->orig_2nd = var;         /* temporarily store var */
      }
      else if (save_cost < lowest2_iy) {
         lowest2_iy = save_cost;
      }
      var = var->next;
   }


   if (lowest_xj == MAX_COST) {
      lowest_xj = lowest2_xj;
   }
   if (lowest_iy == MAX_COST) {
      lowest_iy = lowest2_iy;
   }

/*  Taking Cycle Edge Restriction out, makes it worse for high nodes
 * while it works nicely with  out CERs in dispers2.c
*/
#ifdef DUMB
   /* Cycle Edge Restricitons
    * can't have x and y coming and going to the same part
    */
   if (j->orig_2nd == i->orig_2nd) {
      if (lowest2_xj - lowest_xj > lowest2_iy - lowest_iy) {
         if (lowest2_iy != MAX_COST)
            lowest_iy = lowest2_iy;
      }
      else {
         if (lowest2_xj != MAX_COST)
            lowest_xj = lowest2_xj;
      }
   }
#endif
/*
*/

   judge = (sum_t)lowest_xj + lowest_iy - j->cost_1rst;
#ifdef DPRINT
if (param.verbose)
dump<<"("<<j->tail->id<<")j<-i("<<i->head->id<<") JUDGE "<<judge<<" = "
   <<lowest_xj<<"(xj) + "<<lowest_iy<<"(iy) - "<<j->cost_1rst<<"(ij)\n";
#endif

   if (judge < c->judgement) {
      c->j_part = j;
      c->i_part = i;
      c->judgement = judge;
   }
}

static void extract_edge_from_cycle(cycle_choice_t *c, part_t *cycle_part)
{
   /* now check current_cycle_choice to extract edge from cycle */

   /* check to see if extracted edge is the last edge */
#ifdef DPRINT
if (param.verbose)
dump << "+++++chOse (" << c->j_part->tail->id << ")j->i("
   << c->i_part->head->id << ")\n";
#endif
   if (c->j_part->tail->id == cycle_part->tail->id) {
      /* if so then the edge is already extracted */
#ifdef DPRINT
if (param.verbose)
print_part(cycle_part, 1);
#endif
      return;
   }
   /* otherwise connect cycle_parts head and tail, and split at cycle choice */
   cycle_part->tail->next = cycle_part->head;
   c->j_part->tail->next = NULL;
   cycle_part->head = c->i_part->head;
   cycle_part->tail = c->j_part->tail;

#ifdef DPRINT
if (param.verbose)
print_part(cycle_part, 1);
#endif
}

   
static void init_cycle_choice(cycle_choice_t *c)
{
   c->judgement = SUM_MAX;
   c->j_part = c->i_part = NULL;
}

/* free_decisions
 *
 * frees all the decisions assuming the parts have been freed before
 */
void DispersionHeuristic :: free_decisions()
{
   decision_site_t *dec_var = decisions;

   while (dec_var != NULL) {
      decisions = dec_var->next;
      delete dec_var;
      dec_var = decisions;
   }
}

/* find_minimum()
 *
 */
void DispersionHeuristic :: find_1rst_minimum(part_t *part)
{
// cost_t *costs = map->sites[part->tail->id].in_cost;
   city_id_t partid = part->tail->id;
   cost_t save_cost, lowest = COST_MAX;
   part_t *lowest_part = NULL, *var = parts;

   for (; var != NULL; var = var->next) {
      if (var == part)
         continue;
assert(var->head->id < degree && partid < degree);
      save_cost = matrix->val(var->head->id, partid);
      if (lowest > save_cost) {
         lowest = save_cost;
         lowest_part = var;
      }
   }

   part->orig_1rst = lowest_part;
   part->cost_1rst = lowest;
}

/* generate_head_sites
 * 
 * this procedure makes a first generation list of head sites
 * from the_froms's list, by adding sites to the head site list
 * if they have NO optimal arcs coming from them.
 */

void DispersionHeuristic :: generate_head_sites()
{
   part_t *var;
   int heads=0, x;

   for (var = parts; var != NULL; var = var->next) {
      x = var->head->id;
      assert(x < (int)degree);
      if (the_froms[x].number_of_froms == 0) {
         head_site[heads++].part = var;
      }
   }
   assert(heads <= (int)degree);
   head_sites = heads;
}

/* generate_tail_sites
 *
 * this procedure makes a first generation list of tail sites
 * by looking to see what decide didn't choose (i.e. if the used_2nd
 * was used then there is no arc coming from another site to this
 * part, there for making it a tail site, opposite of a head site.
 *
 * will be called right after decide() returns
void DispersionHeuristic::generate_tail_sites()
{
   part_t *var;
   id_t tails=0;

   tail_sites=0;
   for (var = parts; var != NULL; var = var->next) {
      if (var->used_2nd) {
         var->tail_index = tails;
         tail_site[tails++].part = var;
      }
   }
   tail_sites = tails;
}
 */

/* find_2nd_minimum
 *
 * find the 2nd minimum among the sites which have no to's (among head sites)
 *
 * must call generate_head_sites on the current state before using this
 * call!!!
 */

void DispersionHeuristic :: find_2nd_minimum(part_t *part)
{
// cost_t *costs = map->sites[part->tail->id].in_cost;
   city_id_t partid = part->tail->id, id;
   cost_t save_cost, lowest = COST_MAX;
   part_t *lowest_part = NULL;
   int x=0;
   
   for (x=0; x < head_sites; x++) {
      if ((id=head_site[x].part->head->id) == part->orig_1rst->head->id ||
       id == part->head->id)
         continue;      /* don't count 1rst or your own head site */
      assert(id < degree && partid < degree);
      save_cost = matrix->val(id, partid);
      if (lowest > save_cost) {
         lowest = save_cost;
         lowest_part = head_site[x].part;
      }
   }

   part->orig_2nd = lowest_part;
   part->cost_2nd = lowest;
}

/* add_decision_to *
 * adds a decision_site_t to the multi-from lists and to the decisisons list
 * CeH
 */
void DispersionHeuristic :: add_decision_to(part_t *part)
{
   decision_site_t *dec_var;
   city_id_t the_to_id;

   /* initialize the decision info */
   dec_var = new decision_site_t;
   dec_var->part = part;
   the_to_id = part->orig_1rst->head->id;
   dec_var->used_2nd = &(part->used_2nd);
   part->used_2nd = 0;
   assert(the_to_id < degree);
   the_froms[the_to_id].number_of_froms++;

   /* link to the decisions global */
   if (the_froms[the_to_id].decisions == NULL) {
      dec_var->next = decisions;
      the_froms[the_to_id].decisions = decisions = dec_var;
   }
   else {
      dec_var->next = the_froms[the_to_id].decisions->next;
      the_froms[the_to_id].decisions->next = dec_var;
      if (dec_var == decisions)
         decisions = the_froms[the_to_id].decisions;
   }
}

/* decide
 *
 * Returns the number of parts.  if number of parts are 2 or less than you
 * can call make_path_from_parts and the algorithm is DONE.
 *
 * short explaination:
 * this routine looks at each multi-from site, and the decisions sites paired
 * with it, and chooses the best decisions site.  It disperses the decisions
 * of multi-froms makeing others no longer point to a orig site that other
 * decisions sites want to go to.
 *
 * Explanation
 * A decision site is a site which has choosen its minimum in cost to come from
 * the same site as another decisions site has.  So there are sets of decision
 * sites each set pointing to their multi-from site.
 *
 * the decide preforms the multi-from disperses by looking at a set of
 * decisions sites (each thinking they want to have their arc to be the arc
 * to keep in the DOP.)  For each decision site it computes the difference
 * between the minimum cost of coming from its multi-from site, the 1rstMIC, and
 * another cost which is the cost of coming from one of the head sites, the
 * 2ndMIC.
 *
 * IMPORTANT Note:  This function mearly decides which arc to choose by
 * writing a 1 or 0 in the used_2nd of the sites/parts.  It does not
 * pick the 2ndminimum, nor does it try to construct new parts.  It just
 * decides and doesn't know how it got the 2ndMIC nor act on that decision
 * that it makes.
 */
int DispersionHeuristic :: decide()
{
   part_t *var;
   decision_site_t *dec_var;
   decision_site_t *highest_dec;
   cost_t highest, save_cost;
   city_id_t y,x;
   int number_of_parts=0;

   for (var = parts; var != NULL; var = var->next) {
      number_of_parts++;
      x = var->head->id;
                              /* for every multi-from site a site : a site that
                               * has 2 or more to's in the the_froms list
                               */
      assert(x < degree);
      if (the_froms[x].number_of_froms <= 1)
         continue;

      highest_dec = NULL;
      highest = MIN_COST;
      dec_var = the_froms[x].decisions;
      for (y=0; y < (city_id_t)the_froms[x].number_of_froms; y++) {

            /* for every decision site coming from the
             * multi-from site, find the decision site
             * who's highest difference between the to_cost
             * from the origional to and the next best
             * cost from a _first_generation_ head site
             * to the decision site.
             */

         save_cost = (cost_t)(dec_var->part->cost_2nd
            - dec_var->part->cost_1rst);

         if (save_cost > highest) {
            if (highest_dec != NULL) {
               *highest_dec->used_2nd = 1;
            }
            highest = save_cost;
            highest_dec = dec_var;
         }
         else {
            *dec_var->used_2nd = 1;
         }
         dec_var = dec_var->next;
      }
   }
/*
   generate_tail_sites();
 */
   return number_of_parts;
}

/* cycle extraction
 *
 * makes parts by extracting arcs from all cycles
 *
 *-go through untravled sites (which are the remainging cycles or P-cycles)
 *-and break at the most conveniant site to be a head site for the next
 *-generation
 *
 *
 * Passed a parameter "new_save" which is the list of already compilmentary
 * list of parts.
 *
 *
 * Returns the new list of parts with cycle extractioned parts included.
 */
part_t *DispersionHeuristic :: cycle_extraction(part_t *new_save)
{
   part_t *var, *var_var, *new_parts;
   cycle_choice_t current_cycle_choice;
   int cycles_2nd_pass=0;
   city_id_t head_id;

   init_cycle_choice(&current_cycle_choice);
 
   for (var_var = parts; var_var != NULL; var_var = var_var->next) {
      if (var_var->traveled != NO_ID) {
         continue;
      }

      cycles_2nd_pass++;

      var = var_var;
      head_id = var->head->id;
      new_parts = new part_t;
      new_parts->next = new_save;
      new_save = new_parts;
      new_parts->tail = var->tail;
      new_parts->head = var->head;
      new_parts->traveled = NO_ID;

      update_cycle_choice(&current_cycle_choice,
         /*jpart*/ var, /*ipart*/ var->orig_1rst, new_parts->next);

      for (var=var->orig_1rst; var->head->id != head_id; var=var->orig_1rst) {

/* checking to see if any of the sites with in the cycle have been traveled */
#ifndef NDEBUG
if (var->traveled != NO_ID) {
   dump << "BAD STUFF HERE!  " << var->head->id << "\n";
   assert(0);
}
#endif
         new_parts->tail->next = var->head;
         new_parts->tail = var->tail;

         update_cycle_choice(&current_cycle_choice,
            /*jpart*/ var, /*ipart*/ var->orig_1rst, new_parts->next);

         var->traveled = head_id;
      }

      /* now check current_cycle_choice to extract edge from cycle */
      extract_edge_from_cycle(&current_cycle_choice, new_parts);

      /* and resent the cycle choice */
      init_cycle_choice(&current_cycle_choice);

   }
#ifdef DPRINT
if (param.verbose)
dump << "2nd Pass Cycles " << cycles_2nd_pass << "\n";
#endif
   return new_save;
}

/* make new_parts
 *
 * makes parts from the decisions made from the decide() function
 *
 * and also calls cycle_extraction to make parts from cycles by
 * extraction an arc from each cycle.
 */
void DispersionHeuristic :: make_new_parts()
{
   part_t *new_parts=NULL, *new_save=NULL, *var, *var_var;
   city_id_t head_id;
   int x;

   /*************/
   /* 1rst pass */
   /*************/
   /* make the parts from multi-from disperses, decided by decide()
    *
    *-go through each head site marking the decision parts with which head
    *-they originated from...CC eEH
    */
   for (x=0; x<head_sites; x++) {

      var = head_site[x].part;
      head_id = var->head->id;
      new_parts = new part_t;
      new_parts->next = new_save;
      new_save = new_parts;
      new_parts->tail = var->tail;
      new_parts->head = var->head;
      new_parts->traveled = NO_ID;

      /* really nice loop 
       * to loop through starting with the head site looking for a tail
       * site.   A tail site is where the 2nd orig was used on a part:
       * which means you only want 1rst possibilities in this generation
       * therefore the edge you've come across is going to be decided in
       * the next generation so stop here.
       *
       * A reason we don't want to follow 2nd's is that if we do we might
       * end up coming from our own head site (something I must deal with
       * later, to mask out all 2nd decisions if they come to your own
       * head site and try another head site) which will get this into
       * an infinite loop (well if we actually traveled 2nds :)
       */
      var = head_site[x].part;
      var->traveled = head_id;
      while (var->used_2nd == 0) {

         var=var->orig_1rst;
         var->traveled = head_id;
         new_parts->tail->next = var->head;
         new_parts->tail = var->tail;
      }
   }

   /************/
   /* 2nd pass */
   /************/
   new_parts = cycle_extraction(new_save);

   /* free old parts and clean up the_froms[] */
   for (var = parts; var != NULL;) {
      head_id = var->head->id;
assert(head_id < degree);
      the_froms[head_id].number_of_froms = 0;   /* because decs are freed */
      the_froms[head_id].decisions = NULL;
      var_var = var;
      var = var_var->next;
      delete var_var;
   }

   free_decisions();

   parts = new_parts;      /* new parts is made, so change for find_mins */

   if (new_parts->next == NULL) {   /* only one part close off part and ret */
      new_parts->orig_1rst = new_parts;
      assert(new_parts->head->id < degree && new_parts->tail->id < degree);
      new_parts->cost_1rst =
         matrix->val(new_parts->head->id, new_parts->tail->id);
      new_parts->orig_2nd = NULL;
      new_parts->cost_2nd = COST_MAX;
      return;
   }

   /* find the 1rst and 2nd mins from these guys also making the head_sites */
   for (var = new_parts; var != NULL; var = var->next) {
      find_1rst_minimum(var);
      add_decision_to(var);
   }
   generate_head_sites();
   for (var = parts; var != NULL; var = var->next) {
      find_2nd_minimum(var);
   }
}

/* do_best
 *
 * make the parts into a map that a best branch and bound can solve
 *
 * this changes the orig_1rsts to the best path
 */
// here we have a protectrant that will not cause an infinte loop if
// the dispersion algorihtm is called recurivley
static int recursing_dispersion=0;
void DispersionHeuristic :: do_best()
{
   city_id_t x,y;
   part_t *var=parts, *var_var;
   Matrix nmatrix(things);
   part_t **plist = new part_t *[things];


   for (x=0; x<things; x++, var=var->next) {
      plist[x] = var;
      for (y=0,var_var=parts; y<things; y++, var_var=var_var->next) {
         assert(var_var != NULL && var_var != (void*)0xa0a0a0aL);
         assert(var->head->id < degree && var_var->tail->id < degree);
         nmatrix.cost[x][y] = matrix->val(var->head->id, var_var->tail->id);
      }
      nmatrix.cost[x][x] = MAX_COST;
   }

#ifdef DPRINT
   if (param.verbose) {
      matrix->print();
      nmatrix.print();
   }
#endif
   FindTour *ss;
   Path *p;
   city_id_t a;

   nmatrix.make_type();
   assert(recursing_dispersion < 2);
   recursing_dispersion++;
   ss = new FindTour ("best", &nmatrix, &duration, 0, NULL);
   recursing_dispersion--;

   assert(ss != NULL);
   TourIter ti(*ss->tour);
   while ((p = ti.next()) != NULL) {
      a = p->get_prev()->id;
      plist[p->id]->orig_1rst = plist[a];
   }
   delete ss;
   delete plist;
}

/* make_path_from_parts
 *
 * add sites to the path, corresponding to the parts existing
 * the things parameter is how many parts have been created up to this point.
 */

void DispersionHeuristic :: make_path_from_parts(Tour *tour)
{
   part_t *var=parts, *var_var;
   id_list_t *ivar, *ivar_var;
   part_t *initial_part=var;

#ifdef DPRINT
if (param.verbose) {
   print_parts(parts);
   print_toids(parts, the_froms);
}
#endif
   free_decisions();

   if (things >= 3) {
      do_best();
   }
   do {
      ivar = var->head;
      if (param.verbose > 2)
         dump << matrix->pos[var->head->id] << matrix->pos[var->tail->id];
      while (ivar != NULL) {
         tour->insert_after(NULL, ivar->id);
//       insert_at_head(path, &(map->sites[ivar->id]));
         ivar_var = ivar;
         ivar = ivar->next;
         delete ivar_var;
      }
      var_var = var;
      var = var->orig_1rst;
      delete var_var;
   } while (var != initial_part);
}

int DispersionHeuristic :: run()
{
   int times=0;
   int max_things_left = (recursing_dispersion ? 3 :
      (matrix->is_symmetric() ? MAX_SYM_THINGS_LEFT : MAX_ASYM_THINGS_LEFT));

   if (things > max_things_left) {
      while ((things=decide()) > max_things_left) {
#ifdef DPRINT
if (param.verbose) {
dump << "\nTHINGS=" << things << "\n";
print_parts(parts);
print_toids(parts, the_froms);
}
#endif
         make_new_parts();
         times++;
      }
   }
#ifdef DPRINT
if (param.verbose) {
dump << "THINGS=" << things << "\n";
dump << "DISPERSED=" << times << "\n";
}
#endif
   make_path_from_parts(tour);
   return 0;
}

int DispersionHeuristic :: can_run(const Matrix *m) const
{
   return (m!=NULL);
}

DispersionHeuristic::~DispersionHeuristic()
{
   delete the_froms;
   delete head_site;
}

// after the parts are made this must be called to complete construction
void DispersionHeuristic::construct()
{
   part_t *var;
   decisions = NULL;
   for (var = parts; var != NULL; var = var->next) {
      find_1rst_minimum(var);
      add_decision_to(var);
   }

   /* find a 2nd min */
   generate_head_sites();
   for (var = parts; var != NULL; var=var->next) {
      find_2nd_minimum(var);
   }
}

DispersionHeuristic::DispersionHeuristic (const Matrix *m) : TourFinder(m)
{
   part_t *var=NULL;
   city_id_t x;

   the_froms = new multi_from_t[degree];
   head_site = new head_site_t[degree];

   var = parts = NULL;
   decisions=NULL;
   for(x=0; x<degree; x++) {
      the_froms[x].number_of_froms = 0;
      the_froms[x].decisions = NULL;
      parts = new part_t;
      parts->next = var;
      var = parts;
      parts->tail = parts->head = new id_list_t;
      parts->head->id = x;
      parts->head->next = NULL;
      parts->traveled = NO_ID;
   }
   things = degree;
   construct();
}

DispersionHeuristic::DispersionHeuristic (const Matrix *m, CircList &tourlist)
 : TourFinder(m)
{
   part_t *var=NULL;
   city_id_t x, *idlist = new city_id_t[degree];
   TourListElement *te;
   Path *p;
   id_list_t *id;
   /*
   CircListIter ci(tourlist);  Just Changed cause it's inited below
    */
   CircListIter ci;
   TourIter ti;

   the_froms = new multi_from_t[degree];
   for (x=0; x<degree; x++) {
      idlist[x] = 1;
      the_froms[x].number_of_froms = 0;
      the_froms[x].decisions = NULL;
   }
   for (ci.init(tourlist), x = 0; ci.next() != NULL; x++)
      ;
   things = x;
   head_site = new head_site_t[things];

   var = parts = NULL;
   decisions = NULL;
   ci.init(tourlist);
   while ((te = (TourListElement*)ci.next()) != NULL) {
      parts = new part_t;
      parts->next = var;
      var = parts;
      parts->head = id = NULL;
      for (ti.init(te->tour); (p = ti.next()) != NULL; ) {
#ifdef SHARKIE
dump << p->id << ",";
#endif
         if (parts->head == NULL)
            id = parts->head = new id_list_t;
         else
            id = id->next = new id_list_t;
         id->id = p->id;
#ifdef SHARKIE
if (!idlist[p->id])
dump << p->id << "\n";
#endif
         assert(idlist[p->id]);
         idlist[p->id] = 0;
      }
#ifdef SHARKIE
dump << "\n";
#endif
      if (id == NULL) {
         var = parts->next;
         delete parts;
         parts = var;
      }
      else {
         parts->tail = id;
         parts->tail->next = NULL;
         parts->traveled = NO_ID;
      }
   }
   for (x = 0; x < degree; x++)
      assert(!idlist[x]);
   delete idlist;
   if (things>1)
      construct();
   else
      parts->orig_1rst = parts;
}
