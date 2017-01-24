/* multfrag.cc
 *
 * orig ceh 7-10-94
 */
/*
#define PRINT_IT
 */

#include "multfrag.h"
#include <assert.h>
#include "chnew.h"

SCEntry :: ~SCEntry()
{
#ifdef PRINT_IT
   dump << sc->id << " Dequed\n";
#endif
}

// Point *1*: reverse the path since i'm now pointing in your direction
// Point *2*: terminate the last path and leave startmf in a state where
//            it won't change any labels below since all the labels are changed
// Point *3*: we start at mf_to to assign it specially
//
int MultiFragHeuristic :: run()
{
   city_id_t label_save, j, last_j, lab;
   SCEntry *sc;
   MFLabel *mf, *mf_to, *end=labels+degree, *startmf;
   int sym = matrix->is_symmetric();

   while (count < degree) {
#ifdef PRINT_IT
dump << "Queue\n";
cost_que.print();
#endif
      sc = (SCEntry*)cost_que.deq();
      mf = labels+sc->id;
      mf_to = labels+sc->sc->id;
      switch (mf->degree) {
      case 1: if (sym || mf->to == NO_ID) break;
      case 2: continue;
      }
      if (mf_to->degree == 2 || mf_to->label == mf->label
       || (!sym && mf_to->degree == 1 && mf_to->to == NO_ID))
         sc->enq(cost_que);
      else {
         count++;
#ifdef PRINT_IT
dump << (mf-labels) << "," << (mf_to-labels) << " Edge\n";
#endif
         if (mf->to == NO_ID) {
            if (mf_to->degree == 1 && mf_to->to == NO_ID) { /*1*/
               lab = mf->label;
               assert(sym);
               last_j = NO_ID;
               j = mf_to->label;
               startmf = labels+j;
               for (;;) {
                  startmf->label = lab;
                  label_save = last_j;
                  last_j = j;
                  j = startmf->to;
                  startmf->to = label_save;
                  if (j == NO_ID)
                     break;
                  startmf = labels+j;
               }
               /*2*/
            }
            else
               startmf = mf_to;
            label_save = mf->label;
            mf->to = sc->sc->id;
         }
         else {
            if (mf_to->degree == 1 && mf_to->to != NO_ID) { /*1*/
               assert(sym);
               last_j = (city_id_t)(mf-labels); /*3*/
               j = (city_id_t)(mf_to-labels);
               startmf = mf_to;
               for (;;) {
                  label_save = last_j;
                  last_j = j;
                  j = startmf->to;
                  startmf->to = label_save;
                  if (j == NO_ID)
                     break;
                  startmf = labels+j;
               }
               label_save = last_j;
            }
            else {
               startmf = mf;
               mf_to->to = sc->id;
               label_save = mf_to->label;
            }
         }
         if (++mf->degree < 2)
            sc->enq(cost_que);
         mf_to->degree++;
         assert(mf->degree <= 2);
         assert(mf_to->degree <= 2);
         for (; startmf->to != NO_ID; startmf = labels+startmf->to)
            startmf->label = label_save;
         startmf->label = label_save;
         cleanup((city_id_t)(mf-labels), (city_id_t)(mf_to-labels));
      }
   }

   for (mf = labels; mf<end; mf++) {
      assert(mf->degree > 0);
      if (mf->degree==1 && mf->to != NO_ID)
         break;
   }
   tour->travel((city_id_t)(mf-labels));
   while (mf->to != NO_ID) {
      tour->travel(mf->to);
      mf = labels+mf->to;
   }
   return 0;
}

void MultiFragHeuristic :: cleanup(const city_id_t, const city_id_t)
{
}

void BestInOutHeuristic :: cleanup(const city_id_t i, const city_id_t j)
{
   cost_t *c, **cc, oldcost, newcost;
   SortedCost *wc, **wcc, *e, **ee;
   city_id_t newid = NO_ID;

   sm->cost[i][j].real_cost = MAX_COST;
   if (bestout[i].to == j) {
      oldcost = bestout[i].cost;
      newcost = MAX_COST;
      wc = sm->cost[i];
      for (c = matrix->cost[i], e = wc+degree; wc < e; c++, wc++) {
         if (wc->real_cost != MAX_COST && newcost > *c) {
            newcost = *c;
            newid = (city_id_t)(c-matrix->cost[i]);
         }
      }
      assert(newcost != MAX_COST);
      bestout[i].cost = newcost;
      bestout[i].to = newid;
      for (wc = sm->cost[i]; wc < e; wc++) {
         if (wc->real_cost != MAX_COST)
            wc->real_cost += oldcost - newcost;
      }
      if (bestin[j].to != i)
         sm->resort(i);
   }
   if (bestin[j].to == i) {
      oldcost = bestin[j].cost;
      newcost = MAX_COST;
      wcc = sm->cost;
      for (cc = matrix->cost, ee = wcc+degree; wcc < ee; cc++, wcc++) {
         if ((*wcc)[j].real_cost != MAX_COST && newcost > (*cc)[j]) {
            newcost = (*cc)[j];
            newid = (city_id_t)(cc-matrix->cost);
         }
      }
      assert(newcost != MAX_COST);
      bestin[j].cost = newcost;
      bestin[j].to = newid;
      for (wcc = sm->cost; wcc < ee; wcc++) {
         if ((*wcc)[j].real_cost != MAX_COST)
            (*wcc)[j].real_cost += oldcost - newcost;
      }
      sm->sort(matrix, sm->lambdas);
   }
}

int MultiFragHeuristic :: can_run(const Matrix *m) const
{
   return m!=NULL;
}

MultiFragHeuristic::~MultiFragHeuristic()
{
   delete sm;
   delete labels;
}

BestInOutHeuristic::~BestInOutHeuristic()
{
   if (bestin != NULL) {
      delete bestin;
      bestin = NULL;
   }
}

void MultiFragHeuristic :: construct()
{
   MFLabel *mf, *mfend;
   count = 1;
   cost_que.most = (sum_t)MAX_COST;
   for (mf = labels = new MFLabel[degree], mfend = mf+degree; mf<mfend; mf++) {
      mf->label = (city_id_t)(mf-labels);
      mf->to = NO_ID;
      mf->degree = 0;
      cost_que.enq(new SCEntry(sm->cost, mf->label));
   }
}

MultiFragHeuristic::MultiFragHeuristic (const Matrix *m) : TourFinder(m)
{
   sm = new SortedMatrix(degree);
   sm->sort(m, NO_ID);
   construct();
}

#define MAXBV 1
TransformHeuristic :: TransformHeuristic (const Matrix *m)
 : MultiFragHeuristic(m,0)
{
   Matrix *newm = (Matrix*)m, *writable_matrix = new Matrix(matrix->degree);
   char *trvld = new char[degree], *i, *j, *a, *b, *end;
   cost_t *incosti, *outcostj, ij;
   city_id_t ii, jj;
   double val, bv[MAXBV];
   int x,y;

   newm->make_incosts();
   end = trvld+degree;
   for (i=trvld; i<end; i++)
      *i = 0;
   for (ii=0, i=trvld; i<end; ii++, i++) {
      *i = 1;
      incosti = matrix->incosts[ii];
      for (jj=0, j=trvld; j<end; jj++, j++) {
         if (j == i) {
            writable_matrix->cost[ii][jj] = MAX_COST;
            continue;
         }
         *j = 1;
         for (x=0; x<MAXBV; x++)
            bv[x] = MAX_FLOAT;
         ij = matrix->val(ii, jj);
         for (a=trvld; a<end; a++, incosti++) {
            if (*a)
               continue;
            *a = 1;
            outcostj = matrix->cost[jj];
            for (b=trvld; b<end; b++, outcostj++) {
               if (*b)
                  continue;
               val = ((*incosti + *outcostj)+ij)/3.0;
               for (x=0; x<MAXBV; x++) {
                  if (bv[x] > val) {
                     for (y=MAXBV-1; y>x; y--)
                        bv[y] = bv[y-1];
                     bv[x] = val;
                     break;
                  }
               }
            }
            *a = 0;
         }
         for (x=0, val=0; x<MAXBV; x++) {
            dump << x << " " << bv[x] << "\n";
            val += bv[x]/MAXBV;
         }
         writable_matrix->cost[ii][jj] = (cost_t)(val+.5);
         *j = 0;
      }
      *i = 0;
   }
   delete trvld;
   dump << "Done O(n^4)\n";
   writable_matrix->print();
   sm = new SortedMatrix(degree);
   sm->sort(writable_matrix, NO_ID);
   delete writable_matrix;
   construct();
}

BestInOutHeuristic :: BestInOutHeuristic (const Matrix *m)
 : MultiFragHeuristic(m,0)
{
   cost_t **outcost, **end, *incost, *endi;
   OrderedCost *bi, *bo;
   Matrix *writable_matrix = new Matrix(matrix->degree);

#ifdef NO_ARRAY_CONSTRUCTING
   assert(0);
#endif
   bestin = new OrderedCost[degree*2];
   bestout = bestin+degree;
   outcost = (cost_t**)matrix->cost;
   for (bo = bestout, end = outcost+degree; outcost < end; outcost++, bo++) {
      incost = *outcost;
      for (bi = bestin, endi = incost+degree; incost < endi; incost++, bi++) {
         if (bo->cost > *incost) {
            bo->cost = *incost;
            bo->to = (city_id_t)(incost-*outcost);
         }
         if (bi->cost > *incost) {
            bi->cost = *incost;
            bi->to = (city_id_t)(outcost-matrix->cost);
         }
      }
   }
   outcost = writable_matrix->cost;
   for (bo = bestout, end = outcost+degree; outcost < end; outcost++, bo++) {
      incost = *outcost;
      for (bi = bestin, endi = incost+degree; incost < endi; incost++, bi++) {
         *incost = *incost*3 - bo->cost - bi->cost
            - matrix->val((city_id_t)(incost-*outcost),
               (city_id_t)(outcost-writable_matrix->cost))
         ;
      }
   }
   sm = new SortedMatrix(degree);
   sm->sort(writable_matrix, NO_ID);
   delete writable_matrix;
   construct();
}
