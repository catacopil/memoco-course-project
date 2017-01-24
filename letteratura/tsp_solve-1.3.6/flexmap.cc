/* flexmap.cc
 *
 * Finds a tour according to the paper by Bernd Fritzke and Peter Wilke
 * using the Flexmap algorithm described within, the paper is located in
 * IJCNN-91 Singapore.
 *
 * orig ceh
 */

#include "chlib.h"
#include "flexmap.h"
#include <assert.h>
#include "chnew.h"
#include "rand.h"

#ifndef min
#define min(_a,_b) (((_a)<(_b)) ? (_a) : (_b))
#endif

int FlexMapHeuristic :: run(void)
{
   short x;
   city_id_t r;
   FMCity *fc;
   FMCell *Eworst = NULL, *temp;
   double worst = MIN_FLOAT, err;
   FMCity swap;
   CircListIter ci;

   if (param.verbose>1) {
      city_id_t x;
      for (x=0; x<degree; x++) {
         dump << cities[x].pos << " ";
      }
      dump << "Cities\n";
   }
   for (;;) {
      worst = MIN_FLOAT;
      for (x = 0; x < Ndistribution; x++) {
/*
static num=0;
num++;
 */
         r = rand()%(degree-num_pinned_cities);
         fc = cities + (num_pinned_cities + r);
         if (fc->find_new_bmu(min(cells.size()/2-1, Kneighbor/2))
          && ++fc->pins > Npins) {
            fc->bmu->pinned_city = fc->id;
            swap = *fc;
            if (param.verbose>1)
               dump << "Pinned City " << fc->id << "\n";
            *fc = cities[num_pinned_cities];
            cities[num_pinned_cities++] = swap;
            if (num_pinned_cities == degree)
               break;
            continue;
         }
         fc->move_bmu(cells, Eneighbor, Ebmu);
         if ((err = fc->bmu->error + fc->bmu->get_next()->error) > worst) {
            worst = err;
            Eworst = fc->bmu;
         }
         if ((err = fc->bmu->get_prev()->error + fc->bmu->error) > worst) {
            worst = err;
            Eworst = fc->bmu->get_prev();
         }
      }
      if (x<Ndistribution)
         break;
      temp = new FMCell;
      temp->pos.x = (Eworst->pos.x + Eworst->get_next()->pos.x) / 2.;
      temp->pos.y = (Eworst->pos.y + Eworst->get_next()->pos.y) / 2.;
      if (param.verbose>1) {
         dump << Eworst->pos << " " << temp->pos << " "
            << Eworst->get_next()->pos << "\n";
      }
      cells.insert_after(Eworst, temp);
      temp->error = Eworst->error / 3. + Eworst->get_next()->error / 3.;
      Eworst->error *= 2. / 3.;
      Eworst->get_next()->error *= 2. / 3.;
      if (param.verbose>1) {
         dump << "New Cell " << temp->pos << "\n";
         for (ci.init(cells); (temp = (FMCell*)ci.next()) != NULL; )
            dump << temp->pos << " ";
         dump << "\n";
      }
   }
dump << cells.size() << "\n";
   for (ci.init(cells); (temp = (FMCell*)ci.next()) != NULL; ) {
      if (temp->pinned_city != NO_ID)
         tour->travel(temp->pinned_city);
   }
   return 0;
}

int FlexMapHeuristic :: can_run(const Matrix *m) const
{
   if (!m->is_geometric_2d()) {
      dump << "Can Only Run FlexMap with Euclidean TSPs\n";
      return 0;
   }
   assert(m->pos != NULL);
   return 1;
}

#ifdef SPONGE
#define STARTING_CELLS (degree/4+4)
#else
#define STARTING_CELLS (3)
#endif

// start the flexmap heuristic with a ring of cells around the center of
// gravity of the cities with also spreading the radius of the ring large if
// the cities are more spread out from west, east, north or south of the
// center of gravity.
FlexMapHeuristic::FlexMapHeuristic (const Matrix *m) : TourFinder(m)
{
   city_id_t x;
   double maxy, maxx, miny, minx, gravx, gravy;
   pos_t p;
   FMCell *fc = NULL;

   num_pinned_cities = 0;
   cities = new FMCity [degree];
   if (!can_run(m))
      return;
   Ebmu = .2; //.05
   Eneighbor = .1; //.05
   Ndistribution = 100; // (degree/10)+5; //100
   Kneighbor = 50; // (degree/5)+2; //50
   Npins = 100; // (degree/10)+5; //100
   miny = minx = MAX_FLOAT;
   maxy = maxx = MIN_FLOAT;
   gravx = gravy = 0.0;
   for (x=0; x<degree; x++) {
      cities[x].pos = p = m->pos[x];
      cities[x].id = x;
      gravx += p.x;
      gravy += p.y;
      if (p.x > maxx)
         maxx = p.x;
      if (p.y > maxy)
         maxy = p.y;
      if (p.y < miny)
         miny = p.y;
      if (p.x < minx)
         minx = p.x;
   }
#ifdef SPONGE
   gravx /= (double)degree;
   gravy /= (double)degree;
   double increment = 2*M_PI/(double)STARTING_CELLS;
   double end = 2*M_PI-increment/2, i;
   i = rand()*increment/MAX_RAND;
   for (; i < end; i += increment) {
      fc = new FMCell;
      if (cos(i) < 0)
         fc->pos.x = gravx+(gravx-minx)*cos(i)/2.;
      else
         fc->pos.x = gravx+(maxx-gravx)*cos(i)/2.;
      if (sin(i) < 0)
         fc->pos.y = gravy+(gravy-miny)*sin(i)/2.;
      else
         fc->pos.y = gravy+(maxy-gravy)*sin(i)/2.;
      cells.insert(fc);
   }
#else
   for (x = 0; x < STARTING_CELLS; x++) {
      fc = new FMCell;
      fc->pos.x = minx + rand()*(maxx-minx)/MAX_RAND;
      fc->pos.y = miny + rand()*(maxy-miny)/MAX_RAND;
      cells.insert(fc);
   }
#endif
   for (x=0; x<degree; x++) {
      cities[x].bmu = fc;
      fc = fc->get_next();
   }
}

FlexMapHeuristic::~FlexMapHeuristic()
{
   CircListIter ci;
   FMCell *fc;

   delete cities;
   if (param.verbose) {
      Term t(0);
      t.open("Cells", "w", 0);
      for (ci.init(cells); (fc = (FMCell*)ci.next()) != NULL; ) {
         t << fc->pos;
         delete fc;
      }
      t.close("Cells");
   }
   else {
      for (ci.init(cells); (fc = (FMCell*)ci.next()) != NULL; )
         delete fc;
   }
}

FMCell::FMCell()
{
   initialized = 0;
   error = 0;
   pinned_city = NO_ID;
}

inline double FMCell::operator - (const pos_t &p)
{
   return est_distance(&pos, &p);
}

void FMCell::move_towards(const pos_t &p, const double epsilon)
{
   pos.x += (p.x - pos.x)*epsilon;
   pos.y += (p.y - pos.y)*epsilon;
}

FMCity :: FMCity()
{
   pins = 0;
   bmu = NULL;
}

// returns if the city was pinned the maximum number of times
int FMCity::find_new_bmu(const short half_neighbors)
{
   short x;
   FMCell *n, *p, *oldbmu;
   double best_dist, dist;

   if (bmu->pinned_city != NO_ID)
      best_dist = FLOAT_MAX;
   else
      best_dist = *bmu - pos;
   n = p = oldbmu = bmu;
   for (x = 0; x < half_neighbors; x++) {
      n = n->get_next();
      if (n->pinned_city == NO_ID && (dist = *n - pos) < best_dist) {
         best_dist = dist;
         bmu = n;
      }
      p = p->get_prev();
      if (p->pinned_city == NO_ID && (dist = *p - pos) < best_dist) {
         best_dist = dist;
         bmu = p;
      }
   }
   if (bmu == oldbmu)
      return 1;
   pins = 0;
   return 0;
}

void FMCity::move_bmu(CircList &, const float Eneighbor, const float Ebmu)
{
   if (bmu->get_next()->pinned_city == NO_ID)
      bmu->get_next()->move_towards(pos, Eneighbor);
   if (bmu->get_prev()->pinned_city == NO_ID)
      bmu->get_prev()->move_towards(pos, Eneighbor);
   bmu->error += *bmu - pos;
   bmu->move_towards(pos, Ebmu);
}
