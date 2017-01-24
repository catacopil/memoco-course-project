/* disperse.h
 *
 * algorithm based on the paper by Chad Hurwitz,
 * Traveling Salesperson Dispersion: Performance and Desc. of a Heuristic, 1992
 *
 * Orig Nov 26, 1991 Chad Hurwitz
 */
#ifndef _DISPERSE_H
#define _DISPERSE_H

#include "tourfind.h"

typedef struct id_list_t {
   city_id_t id;
   struct id_list_t *next;
} id_list_t;

typedef struct part_t {
   id_list_t *head;              // site where this parts path begins
   id_list_t *tail;              // site where this parts path ends
   cost_t cost_1rst;             // The minimum from the tail site
   struct part_t *orig_1rst;     // The origin site where lowest min is from
   cost_t cost_2nd;              // the 2nd min in cost from tail sites
   struct part_t *orig_2nd;      // the "head", i.e. froms==0
                                 // site of origin of 2nd
   struct part_t *next;          // the next part in the parts list
   short used_2nd;               // if part will choose 1rst or 2nd
   city_id_t traveled;           // if part has been traveled so far, this
                                 // field will show the next generation
                                 // part's head site id
   short temp;
   // for cycles parts only
   city_id_t head_index;         // index of cycles head site in head_sites
   city_id_t tail_index;         // index of cycles tail site in tail_sites
} part_t;

typedef struct decision_site_t {
   part_t *part;                 // the part which is making the "to?" decision
   short *used_2nd;              // if this decision chose to use the 2nd best
   struct decision_site_t *next; // pointer to the next decision
} decision_site_t;

typedef struct {
   decision_site_t *decisions;
   int number_of_froms;
} multi_from_t;

typedef struct {
   part_t *part;
} head_site_t;

typedef struct {
   part_t *part;
} tail_site_t;

typedef struct cycle_choice_t {
   part_t      *j_part;          // possible tail part of cycle
   part_t      *i_part;          // possible head of cycle
   sum_t       judgement;        // the judgement cost cxj+ciy-cij */
} cycle_choice_t;

class DispersionHeuristic : public TourFinder
{
   int things;
   // a list of parts created so far
   part_t *parts;

   // a list of decisions
   decision_site_t *decisions;

   // used to find the head sites (sites with NO sites min costs
   // coming from them)
   multi_from_t *the_froms;
   head_site_t *head_site;
   int head_sites;

   void free_decisions();
   void find_1rst_minimum(part_t *part);
   void find_2nd_minimum(part_t *part);
   void generate_head_sites();
   void add_decision_to(part_t *part);
   int decide();
   part_t *cycle_extraction(part_t *new_save);
   void make_new_parts();
   void do_best();
   void make_path_from_parts(Tour *tour);
   void update_cycle_choice(cycle_choice_t *c,
      part_t *j, part_t *i, part_t *parts);
   void construct();

public:
   DispersionHeuristic(const Matrix*, CircList &tourlist);
   DispersionHeuristic(const Matrix*);
   ~DispersionHeuristic();
   int can_run(const Matrix *) const;
   int run();
};

#endif
