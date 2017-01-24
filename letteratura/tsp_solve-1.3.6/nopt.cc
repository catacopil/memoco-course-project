/* nopt.cc
 *
 * for asymmetric TSPs, swap N-tuple edges in a tour if can be made better.
 * including the r-opt algorithm by Lin and Kernighan
 *
 * orig ceh 6-1-94
 */
#define MAXP 100

#define KSWAPSIZE 10
#define KSWAPINC 2

/*
#define CHECKIT
#define PRINTIT
#define PRINTITK
 */

#include "nopt.h"
#include "findtour.h"
#include <assert.h>
#include "rand.h"

#define SWAPX  0x01
#define SWAPY  0x02
#define SWAPXY 0x04
#define SWAPZ  0x08
#define SWAPXZ 0x10
#define SWAPYZ 0x20

void NOptHeuristic :: swap4(city_id_t *x, city_id_t *y, city_id_t *z,
   city_id_t *, int swap)
{
   city_id_t i;
#ifdef PRINTIT
if (param.verbose) {
sum_t j=0;
for (i=0; i<degree+1; i++) {
   dump << tourcopy[i] << " ";
   if (i<degree)
   j += matrix->val(tourcopy[i], tourcopy[i+1]);
}
dump << j << " Before\n";
}
#endif
   if (swap&SWAPX) {
#ifdef PRINTIT
if (param.verbose)
dump << "Swapig X, ";
#endif
      for (i=0; i < (y-x)/2; i++)
         xtracopy[i] = x[i+1];
      for (i=0; i < (y-x)/2; i++)
         x[i+1] = x[(int)(y-x)-i];
      for (i=0; i < (y-x)/2; i++)
         x[(int)(y-x)-i] = xtracopy[i];
   }
   if (swap&SWAPY) {
#ifdef PRINTIT
if (param.verbose)
dump << "Swapig Y, ";
#endif
      for (i=0; i < (z-y)/2; i++)
         xtracopy[i] = y[i+1];
      for (i=0; i < (z-y)/2; i++)
         y[i+1] = y[(int)(z-y)-i];
      for (i=0; i < (z-y)/2; i++)
         y[(int)(z-y)-i] = xtracopy[i];
   }
   if (swap&SWAPXY) {
#ifdef PRINTIT
if (param.verbose)
dump << "Swapig XY, ";
#endif
      for (i=0; i < (y-x); i++)
         xtracopy[i] = x[i+1];
      for (i=0; i < (z-y); i++)
         x[i+1] = y[i+1];
      for (i=0; i < (y-x); i++)
         z[i+1-(city_id_t)(y-x)] = xtracopy[i];
   }
#ifdef PRINTIT
if (param.verbose)
dump << "\n";
#endif
   tourcopy[degree] = tourcopy[0];
#ifdef PRINTIT
if (param.verbose) {
sum_t j=0;
for (i=0; i<degree+1; i++) {
   dump << tourcopy[i] << " ";
   if (i<degree)
   j += matrix->val(tourcopy[i], tourcopy[i+1]);
}
dump << j << " After\n";
}
#endif
}

#ifdef PRINTIT
static int SUM_GREATER(sum_t a, sum_t b)
{
   if (a > b) {
      if (param.verbose)
         dump << (a-b) << " J\n";
      return 1;
   }
   return 0;
}
//#define SWAP4(a, b, c, d, e, f) swap4(a, b, c, d, e)
#else
#define SUM_GREATER(a, b) ((a)>(b))
#endif

#ifdef CHECKIT
void NOptHeuristic :: SWAP4(city_id_t *a, city_id_t *b, city_id_t *c,
   city_id_t *d, int e, sum_t diff)
{
   sum_t s1, s2;
   city_id_t i1;
   assert(diff>0);
   for (s1=0, i1=0; i1<degree; i1++) { 
      assert(tourcopy[i1] < degree);
      assert(tourcopy[i1+1] < degree);
      s1 += matrix->cost[tourcopy[i1]][tourcopy[i1+1]];
   }
   swap4(a, b, c, d, e);
   for (s2=0, i1=0; i1<degree; i1++) {
      assert(tourcopy[i1] < degree);
      assert(tourcopy[i1+1] < degree);
      s2 += matrix->cost[tourcopy[i1]][tourcopy[i1+1]];
   }
   if (!fequiv(s1-s2, diff, .1))
      dump << s1 << "-" << s2 << " <> " << diff << "\n";
}
#else
#define SWAP4(a, b, c, d, e, f) swap4(a, b, c, d, e)
#endif

/* There are 7 possible swas for 3-opting symmetric tours and 1 possible
 * swap for 3-opting asymmetric tours.  [A] is the swap that can work with
 * both types.  [B] [C] [D] are really only 2-opt swaps but since we have
 * the opportunity to do them at the time of swap comparison, they are
 * included.
 *
 *    1       2       3                1       3       2
 * ------> ------> ------>          ------> ------> ------>    [A]
 * z[1] *x x[1] *y y[1] *z          z[1] *x y[1] *z x[1] *y
 *
 *    1       2       3                1       3       2
 * ------> <------ ------>   [B]    ------> <------ ------>    [E]
 * z[1] *x *y x[1] y[1] *z          z[1] *x *z y[1] x[1] *y
 *
 *    1       2       3                1       3       2
 * ------> ------> <------   [C]    ------> ------> <------    [F]
 * z[1] *x x[1] *y *z y[1]          z[1] *x y[1] *z *y x[1]
 *
 *    1       3       2                1       2       3
 * ------> <------ <------   [D]    ------> <------ <------    [G]
 * z[1] *x *z y[1] *y x[1]          z[1] *x *y x[1] *z y[1]
 */
void NOptHeuristic :: three_opt()
{
   city_id_t *x, *y, *z, *end;
   sum_t cursum, sum_0, sum_1, sum_2, sum_3, sum_4, sum_5=0;
   sum_t sum_6, sum_7, sum_8, sum_9, sum_10, sum_11=0;
   int sym=matrix->is_symmetric(), swap, changed, noni;
#ifdef CHECKIT
   sum_t oldsum;
#endif

   noni = changed = 0;
   end = tourcopy+degree;
   for (x = tourcopy; ; x++) {
      if (changed)
         noni = 0;
      else
         noni++;
      if (noni == degree*3)
         break;
      if (x == end) {
         rand_tourcopy();
         x = tourcopy;
      }
      sum_0 = matrix->val(*x, x[1]);
      changed = 0;
      for (y=x+1; y<end; y++) {
         sum_1 = sum_0 + (sum_3=matrix->val(*y, y[1]));
         sum_2 = matrix->val(*x, y[1]);
         if (sym && SUM_GREATER(sum_1, (sum_11 = matrix->val(*x, *y))
          + (sum_5 = matrix->val(x[1], y[1]))) ) { // [B]
            SWAP4(x, y, NULL, NULL, SWAPX, sum_1 - sum_11 - sum_5);
            sum_0 = matrix->val(*x, x[1]);
            sum_1 = sum_0 + (sum_3=matrix->val(*y, y[1]));
            //sum_2 will stay the same
            sum_5 = matrix->val(x[1], y[1]);
            sum_11 = matrix->val(*x, *y);
            changed = 1;
         }
         for (z=y+1; z<end; z++) {
#ifdef PRINTIT
if (param.verbose) {
dump << (x-tourcopy) << ", " << (y-tourcopy) << ", " << (z-tourcopy) << " -- "
<< (sum_1 + matrix->val(*z, z[1])) << " > "
<< (sum_2 + matrix->val(*y, z[1]) + matrix->val(*z, x[1]) ) << "\n";
}
#endif
#ifdef CHECKIT
oldsum =
#endif
            cursum = sum_1 + matrix->val(*z, z[1]);
            if (sym) {
               swap = 0;
               sum_4 = matrix->val(*y, z[1]);
               sum_7 = matrix->val(*y, *z);
               sum_6 = matrix->val(*x, *z);
               sum_8 = matrix->val(x[1], z[1]);
               sum_9 = matrix->val(y[1], z[1]);
               sum_10 = matrix->val(*z, x[1]);
               if (SUM_GREATER(cursum, sum_2+sum_4+sum_10)) { // [A]
                  cursum = sum_2+sum_4+sum_10;
                  swap = SWAPXY;
               }
               if (SUM_GREATER(cursum, sum_0+sum_7+sum_9)) { // [C]
                  cursum = sum_0+sum_7+sum_9;
                  swap = SWAPY;
               }
               if (SUM_GREATER(cursum, sum_3+sum_6+sum_8)) { // [D]
                  cursum = sum_3+sum_6+sum_8;
                  swap = SWAPY|SWAPX|SWAPXY;
               }
               if (SUM_GREATER(cursum, sum_4+sum_5+sum_6)) { // [E]
                  cursum = sum_4+sum_5+sum_6;
                  swap = SWAPY|SWAPXY;
               }
               if (SUM_GREATER(cursum, sum_2+sum_7+sum_8)) { // [F]
                  cursum = sum_2+sum_7+sum_8;
                  swap = SWAPX|SWAPXY;
               }
               if (SUM_GREATER(cursum, sum_11+sum_10+sum_9)) { // [G]
                  cursum = sum_11+sum_10+sum_9;
                  swap = SWAPX|SWAPY;
               }
               if (swap) {
                  SWAP4(x, y, z, z, swap, oldsum-cursum);
                  sum_0 = matrix->val(*x, x[1]);
                  sum_1 = sum_0 + (sum_3=matrix->val(*y, y[1]));
                  sum_2 = matrix->val(*x, y[1]);
                  sum_5 = matrix->val(x[1], y[1]);
                  sum_11 = matrix->val(*x, *y);
                  changed = 1;
               }
            }
            else {
               if (SUM_GREATER(cursum,
                sum_2 + matrix->val(*y, z[1]) + matrix->val(*z, x[1]))) {
                  SWAP4(x, y, z, z, SWAPXY, cursum - (sum_2
                   + matrix->val(*y, z[1]) + matrix->val(*z, x[1])));
                  sum_0 = matrix->val(*x, x[1]);
                  sum_1 = sum_0 + (sum_3=matrix->val(*y, y[1]));
                  sum_2 = matrix->val(*x, y[1]);
                  changed = 1;
               }
            }
         }
      }
   }
}

void NOptHeuristic :: four_opt()
{
   city_id_t i, *x, *y, *z, *w, *end, *c;
   sum_t sum_0, sum_1, sum_2, sum_01;
   cost_t *ycost;

   end = tourcopy+degree;
   for (x=tourcopy; x<end; x++) {
      sum_0 = matrix->val(*x, x[1]);
      for (y=x+3; y<end; y++) {
         ycost = matrix->cost[*y];
         sum_01 = sum_0 + ycost[y[1]];
         for (z=y+3; z<end; z++) {
            sum_1 = sum_01 + matrix->val(*z, z[1]);
            sum_2 = matrix->val(*x, z[1]) + matrix->val(*z, x[1]);
            for (w=z+3; w<end; w++) {
               if (sum_1 + matrix->val(*w, w[1]) > sum_2
                + ycost[w[1]] + matrix->val(*w, y[1])) {
                  for (i=0; i < (w-z); i++)
                     xtracopy[i] = z[i+1];
                  for (c=xtracopy+i, i=0; i < (z-y); i++)
                     c[i] = y[i+1];
                  for (c+=i, i=0; i < (y-x); i++)
                     c[i] = x[i+1];
                  memcpy(x+1,xtracopy, sizeof(city_id_t)*(int)((c+i)-xtracopy));
                  tourcopy[degree] = tourcopy[0];
                  sum_0 = matrix->val(*x, x[1]);
                  ycost = matrix->cost[*y];
                  sum_01 = sum_0 + ycost[y[1]];
                  sum_1 = sum_01 + matrix->val(*z, z[1]);
                  sum_2 = matrix->val(*x, z[1]) + matrix->val(*z, x[1]);
               }
            }
         }
      }
   }
}

#ifdef PRINTIT

#define PLOPIT() \
if (param.verbose) { \
for (tmp=0, i=0; i<degree; i++) { \
   dump << tourcopy[i] << " "; \
   tmp += cost[tourcopy[i]][tourcopy[i+1]]; \
} \
dump << " " << tmp << " Before\n"; \
} \

#define SLOPIT(_sum) \
if (param.verbose) { \
dump << base_sum << " > " << (_sum) << "\n"; \
for (tmp=0, i=0; i<degree; i++) { \
   dump << tourcopy[i] << " "; \
   tmp += cost[tourcopy[i]][tourcopy[i+1]]; \
} \
dump << " " << tmp << " After\n"; \
} \

#else
#define PLOPIT()
#define SLOPIT(_sum)
#endif

#define FIVE_SWAP(_sum) \
   memcpy(x+1, xtracopy, sizeof(city_id_t)*(int)((c+i)-xtracopy)); \
   tourcopy[degree] = tourcopy[0]; \
   xcost = cost[*x]; \
   base_0 = xcost[x[1]]; \
   ycost = cost[*y]; \
   base_1 = base_0 + ycost[y[1]]; \
   sum_3_0 = xcost[y[1]]; \
   zcost = cost[*z]; \
   base_2 = base_1 + zcost[z[1]]; \
   sum_3_1 = sum_3_0 + ycost[z[1]]; \
   sum_2_0 = xcost[z[1]]; \
   sum_1_0 = zcost[x[1]]; \
   wcost = cost[*w]; \
   base_3 = base_2 + wcost[w[1]]; \
   sum_3_2 = sum_3_1 + zcost[w[1]]; \
   sum_2_1 = sum_2_0 + ycost[w[1]] + wcost[x[1]]; \
   sum_1_1 = sum_1_0 + wcost[y[1]] + xcost[w[1]]; \
   SLOPIT(_sum);

void NOptHeuristic :: five_opt()
{
   city_id_t i, *x, *y, *z, *w, *v, *end, *c;
   cost_t *xcost, *ycost, *zcost, *wcost, *vcost, **cost = matrix->cost;
   sum_t sum_1_0, sum_1_1;
   sum_t sum_2_0, sum_2_1;
   sum_t sum_3_0, sum_3_1, sum_3_2;
   sum_t base_0, base_1, base_2, base_3;
   sum_t base_sum, sum_1, sum_2, sum_3;
#ifdef PRINTIT
   sum_t tmp;
#endif

   end = tourcopy+degree;
   for (x=tourcopy; x<end; x++) {
      xcost = cost[*x];
      base_0 = xcost[x[1]];
      for (y=x+4; y<end; y++) {
         ycost = cost[*y];
         base_1 = base_0 + ycost[y[1]];
         sum_3_0 = xcost[y[1]];
         for (z=y+4; z<end; z++) {
            zcost = cost[*z];
            base_2 = base_1 + zcost[z[1]];
            sum_3_1 = sum_3_0 + ycost[z[1]];
            sum_2_0 = xcost[z[1]];
            sum_1_0 = zcost[x[1]];
            for (w=z+4; w<end; w++) {
               wcost = cost[*w];
               base_3 = base_2 + wcost[w[1]];
               sum_3_2 = sum_3_1 + zcost[w[1]];
               sum_2_1 = sum_2_0 + ycost[w[1]] + wcost[x[1]];
               sum_1_1 = sum_1_0 + wcost[y[1]] + xcost[w[1]];
               for (v=w+4; v<end; v++) {
                  vcost = cost[*v];
                  base_sum = base_3 + vcost[v[1]];
                  sum_3 = sum_3_2 + wcost[v[1]] + vcost[x[1]];
                  sum_2 = sum_2_1 + zcost[v[1]] + vcost[y[1]];
                  sum_1 = sum_1_1 + vcost[z[1]] + ycost[v[1]];
                  if (base_sum > sum_1 && sum_1 < sum_2 && sum_1 < sum_3) {
                     PLOPIT();
                     for (i=0; i < (v-w); i++)
                        xtracopy[i] = w[i+1];
                     for (c=xtracopy+i, i=0; i < (w-z); i++)
                        c[i] = z[i+1];
                     for (c+=i, i=0; i < (z-y); i++)
                        c[i] = y[i+1];
                     for (c+=i, i=0; i < (y-x); i++)
                        c[i] = x[i+1];
                     FIVE_SWAP(sum_1);
                  }
                  else if (base_sum > sum_2 && sum_2 < sum_3) {
                     PLOPIT();
                     for (i=0; i < (w-z); i++)
                        xtracopy[i] = z[i+1];
                     for (c=xtracopy+i, i=0; i < (y-x); i++)
                        c[i] = x[i+1];
                     for (c+=i, i=0; i < (v-w); i++)
                        c[i] = w[i+1];
                     for (c+=i, i=0; i < (z-y); i++)
                        c[i] = y[i+1];
                     FIVE_SWAP(sum_2);
                  }
                  else if (base_sum > sum_3) {
                     PLOPIT();
                     for (i=0; i < (z-y); i++)
                        xtracopy[i] = y[i+1];
                     for (c=xtracopy+i, i=0; i < (v-w); i++)
                        c[i] = w[i+1];
                     for (c+=i, i=0; i < (y-x); i++)
                        c[i] = x[i+1];
                     for (c+=i, i=0; i < (w-z); i++)
                        c[i] = z[i+1];
                     FIVE_SWAP(sum_3);
                  }
               }
            }
         }
      }
   }
}

void NOptHeuristic :: rand_tourcopy()
{
   city_id_t *t1, *t2, *end, i = rand()%(degree-1)+1;

   for (t1 = tourcopy, t2 = xtracopy, end = tourcopy+i; t1<end; )
      *t2++ = *t1++;
   for (t1 = tourcopy+i, t2 = tourcopy, end = tourcopy+degree; t1<end; )
      *t2++ = *t1++;
   for (t1 = xtracopy, t2 = end-i; t2<end; )
      *t2++ = *t1++;
   tourcopy[degree] = *tourcopy;
}

// searches for the edge [i,j], that will (after inserting [k,i])
// remove the cycle if detached.  Also swaps (revserses).
// the path from K to J will be reversed to permit the swap.
//
// This will then remove [l,k], insert [k,i], remove [i,j] and insert [j,l]
// which will keep the tour together.  And return j.
//
// Point *1*: assigns forward[l] = j, truely
//
city_id_t NOptHeuristic :: find_j_and_swap(city_id_t *k, const city_id_t i)
{
   city_id_t j, last_j = *k, lj, *forwardk = xtracopy+last_j;

/*
for (forwardk = xtracopy, j=0; j<degree; j++, forwardk = xtracopy+*forwardk)
 assert(tourcopy[*forwardk] == (city_id_t)(forwardk-xtracopy));
forwardk = xtracopy+last_j;
 */
   j = *forwardk;
   *forwardk = i;
   tourcopy[i] = last_j;
   for (forwardk = xtracopy+j; j != i; forwardk = xtracopy+j) {
      lj = last_j;
      tourcopy[lj] = last_j = j;
      j = *forwardk;
      *forwardk = lj;
   }
   tourcopy[last_j] = (city_id_t)(k-xtracopy);
   *k = last_j; /*1*/
/*
for (forwardk = xtracopy, j=0; j<degree; j++, forwardk = xtracopy+*forwardk)
 assert(tourcopy[*forwardk] == (city_id_t)(forwardk-xtracopy));
 */
   return *k = last_j; /*1*/
}

void NOptHeuristic :: printr(city_id_t *x)
{
   city_id_t i, j;
   sum_t sum=0;

   for (i=j=0; i<degree; i++) {
      sum += matrix->val(j,x[j]);
      dump << (j=x[j]) << " ";
   }
   dump << "= " << sum << "\n";
}

// Lin and Kernighan's r-opt algorithm [1973]
//
//     K      J     The obvious edges to connect these are IK, JM, LN, but the
//     .------.     real edges are IJ, KL, MN.  When this algorithm gets to L,
//   I.        .M   it finds there is a shortest edge KI, that maximizes gp.
//     \      /     Then it finds J with find_and_swap() and removes KL and IJ,
//      \.  ./      and adds KI and JL.  JL is an edge that will be optimized,
//       N  L       since LJ can be maximized as could LK origonally.
//
// Point *0*: xtracopy is now a "to city" index tour and tourcopy is backwards
//
// Point *1*: minmize the cost back to the city you are removing the edge from
// along with the uniquly determined edge [i,j] cost that must be removed
//
// Point *2*: restrict edges from being added as not previously removed
//
// Point *3*: restrict edges from being removed as not previously added.  The
// added array entry can be NO_ID which means there are no previously added
// edges from the indexed city, MAX_DEGREE which means you may not have any
// more newly added edges on this city because there are already two.  And
// a city_id value means that city is adjecent to the indexed city as a
// previously added edge.
//
// Point *4*: after j is found, then update the added and removed lists.
//
// GS[p] == (G*p - Cjl)
//
// Implementation note: I tried sorting the costs but it doesn't include the
// backwards edge cost and it seemed to make it slower. 
//
void NOptHeuristic :: ropt()
{
   city_id_t *kptr, *end, *tadd, *trem, *besttour=new city_id_t[degree*3];
   city_id_t *added=besttour+degree, *removed=added+degree, *bptr;
   city_id_t j, i=NO_ID, ii=NO_ID, k, last_l, startl, endl, removed_k, downk;
   sum_t GS, gp, bestGS=MIN_SUM;
   cost_t *cost, mincost, tmpcost;
   int p;

   for (kptr = tourcopy, end = kptr+degree; kptr < end; kptr++) /*0*/
      xtracopy[*kptr] = kptr[1];
   memcpy(besttour, xtracopy, sizeof(city_id_t)*degree);
   endl = degree-1;
   for (startl = 0; startl != endl; startl = (startl+1)%degree) {
      for (end=(kptr=xtracopy)+degree, tadd=added, trem=removed; kptr < end; ) {
			/*
         tourcopy[*kptr] = (city_id_t)((kptr++)-xtracopy);
			 */
         tourcopy[*kptr] = (city_id_t)(kptr-xtracopy);
			kptr++;
         *tadd++ = *trem++ = NO_ID;
      }
      kptr = xtracopy + startl;
#ifdef PRINTIT
j = NO_ID; // optimize god
if (param.verbose) {
dump << "Starting City " << startl << "->" << *kptr << "\n";
printr(xtracopy);
}
#endif
      removed[*kptr] = last_l = startl;
      for (p = 0, gp = 0, bestGS = -SUM_GRANU*2; gp > bestGS; p++) {
         k = *kptr;
         mincost = MAX_COST;
         removed_k = removed[k];
         tadd = added;
         trem = removed;
         cost = matrix->cost[k];
         bptr = tourcopy;
         downk = xtracopy[k];
         for (ii = 0; ii < degree; ii++, tadd++, trem++, cost++, bptr++) {
            tmpcost = *cost - matrix->val(ii,*bptr); /*1*/
            if (mincost > tmpcost) {                                    /*2*/
               if (ii != k && ii != downk && ii != removed_k && k != *trem) {
                  if (*tadd == MAX_DEGREE) /*3*/
#ifdef PRINTIT
{
if (param.verbose) {
dump << "FULL CANT REM " << *bptr << "->" << ii << "\n";
}
}
#else
                     ;
#endif
                  else if (*tadd == NO_ID || *tadd != *bptr) {
                     mincost = tmpcost;
                     i = ii;
                  }
#ifdef PRINTIT
else if (param.verbose) {
dump << "SAME CANT REMOVE " << tourcopy[ii] << "->" << ii << "\n";
}
#endif
               }
#ifdef PRINTIT
else if (param.verbose) {
dump << "CANT ADD " << k << "->" << ii << "\n";
}
#endif
            }
         }
         if (mincost == MAX_COST || i == startl)
            break;
         gp += matrix->val(last_l, k) - matrix->val(k, i);
         last_l = i;
         downk = tourcopy[i];
         j = find_j_and_swap(kptr, i);
         assert(j == downk);
#ifdef PRINTIT
if (param.verbose) {
dump << "Added [" << i << "," << k << "]\n";
dump << "Remed [" << i << "," << j << "]\n";
}
#endif
         if (removed[i] == NO_ID) /*5*/
            removed[i] = j;
         else
            removed[j] = i;
         if (added[i] == NO_ID)
            added[i] = k;
         else
            added[i] = MAX_DEGREE;
         if (added[k] == NO_ID)
            added[k] = i;
         else
            added[k] = MAX_DEGREE;
            
         GS = gp + matrix->val(i, j) - matrix->val(j, startl);
#ifdef PRINTIT
if (param.verbose) {
dump << gp << " " << GS << "  ~  ";
printr(xtracopy);
}
#endif
         if (bestGS < GS && GS > 0) {
#ifdef PRINTIT
            dump << "FOUND BETTER " << bestGS << " < " << GS << "\n";
            printr(besttour);
            printr(xtracopy);
#endif
            bestGS = GS;
            memcpy(besttour, xtracopy, sizeof(city_id_t)*degree);
         }
         if (p >= MAXP && (gp <= 0 || tourcopy[j] || gp <= bestGS))
            break;
      }
#ifdef PRINTIT
if (param.verbose) {
dump << "[" << j << "," << startl << "]\n";
}
#endif
      if (bestGS > 0)
         endl = startl;
      memcpy(xtracopy, besttour, sizeof(city_id_t)*degree);
   }
   for (i=j=0; i<degree; i++)
      j = tourcopy[i] = besttour[j];
   tourcopy[degree] = tourcopy[0];
   delete besttour;
}

NOptHeuristic :: NOptHeuristic (const Matrix *m, Tour *t, int tup)
   : TourFinder (m)
{
   city_id_t i;
   TourIter ti(*t);

   xtracopy = new city_id_t[degree+1+degree];
   tourcopy = xtracopy+degree;
   for (i = 0; i < degree; tourcopy[i++] = ti.next()->id)
      ;
   tourcopy[degree] = tourcopy[0];
   switch (tuple = tup) {
   case 2: k3swap(); break;
   case 3: three_opt(); break;
   case 4: four_opt(); break;
   case 5: five_opt(); break;
   case ROPTH: three_opt();
   case ROPT_TUPLE: ropt(); break;
   case KSWAP10_TUPLE: kswap(0); break;
   case SKSWAP10_TUPLE: kswap(1); break;
   default: assert(0);
   }
}

NOptHeuristic :: ~NOptHeuristic()
{
   delete xtracopy;
}

int NOptHeuristic :: can_run(const Matrix *m) const
{
   if (m->degree < 5) {
      dump << "Must have " << 5 << " cities to run.\n";
      return 0;
   }
   if (KSWAP10_TUPLE == tuple && m->degree <= KSWAPSIZE) {
      dump << "Must larger than " << KSWAPSIZE << " to Kswap.\n";
      return 0;
   }
   if (ROPT_TUPLE == tuple && !m->is_symmetric()) {
      dump << "Must Symmetric TSP For R-Opt.\n";
      return 0;
   }
   return 1;
}

#define TRY(_a) (tourcopy[(a+_a)%degree])

void NOptHeuristic :: swap_it(city_id_t *p1, city_id_t *p2)
{
   city_id_t sw;

   sw = *p1;
   *p1 = *p2;
   *p2 = sw;
}

/* noni are the tallied non-improvment moves, if we've cycled the path with
 * no improvements then exit.
 */
#define acost (Cost[0])
#define bcost (Cost[1])
#define ccost (Cost[2])
#define dcost (Cost[3])
#define ecost (Cost[4])
#define acity (City[0])
#define bcity (City[1])
#define ccity (City[2])
#define dcity (City[3])
#define ecity (City[4])
void NOptHeuristic :: k3swap()
{
   city_id_t noni, b, c, d, e, *City[5], *endcity = tourcopy+(degree-5);
   sum_t cost, save, diff;
   cost_t *Cost[5], **begcost = matrix->cost;
   int swap;

   ecity = (dcity = (ccity = (bcity = (acity = tourcopy)+1)+1)+1)+1;
   for (noni = 0; noni<degree; acity++, bcity++, ccity++, dcity++, ecity++) {
      if (City[0] > endcity) {
         assert(City[5-(int)(City[0]-endcity)] == tourcopy+degree);
         City[5-(int)(City[0]-endcity)] = tourcopy;
      }
      swap = 0;
      acost = begcost[*acity]; bcost = begcost[b = *bcity];
      ccost = begcost[c = *ccity]; dcost = begcost[d = *dcity];
      ecost = begcost[e = *ecity];
      save = acost[b]+bcost[c]+ccost[d]+dcost[e]; // 0, 1, 2, 3, 4
      diff = 0;
      /* Seems to work better with out this check
      if ((cost = save - acost[b]-bcost[d]-dcost[c]-ccost[e]) > diff) {
         diff = cost;
         swap = SWAPYZ; // 0, 1, 3, 2, 4
      }
       */
      if ((cost = save - acost[c]-ccost[b]-bcost[d]-dcost[e]) > diff) {
         diff = cost;
         swap = SWAPXY; // 0, 2, 1, 3, 4
      }
      if ((cost = save - acost[c]-ccost[d]-dcost[b]-bcost[e]) > diff) {
         diff = cost;
         swap = SWAPXY|SWAPYZ; // 0, 2, 3, 1, 4
      }
      if ((cost = save - acost[d]-dcost[c]-ccost[b]-bcost[e]) > diff) {
         diff = cost;
         swap = SWAPXZ; // 0, 3, 2, 1, 4
      }
      if ((cost = save - acost[d]-dcost[b]-bcost[c]-ccost[e]) > diff) {
         diff = cost;
         swap = SWAPXY|SWAPXZ; // 0, 3, 1, 2, 4
      }
      if (swap) {
         if (swap & SWAPXY)
            swap_it(bcity, ccity);
         if (swap & SWAPYZ)
            swap_it(ccity, dcity);
         if (swap & SWAPXZ)
            swap_it(bcity, dcity);
         noni = 0;
      }
      else
         noni++;
   }
}

int NOptHeuristic :: run()
{
   city_id_t i;
   for (i=0; i<degree; i++)
      tour->travel(tourcopy[i]);
   return 0;
}

// look at all sequences of k length and optimize with best heuristic
//
// Point *0*: force the first city to be traveled after the last
//
void NOptHeuristic :: kswap(int slow)
{
   city_id_t r, s, i, t;
   Matrix m(KSWAPSIZE);
   cost_t *x, *mx;
   FindTour *ft;
   TourIter ti;
   Path *p;
   int found_better, nobetters, stop = 1+(degree/KSWAPINC);

   found_better = nobetters = 0;
   for (i = 0; i < degree; i++)
      xtracopy[i] = tourcopy[i];
   x = m.cost[KSWAPSIZE-1];
   for (r = 0; r < KSWAPSIZE; r++)
      x[r] = MAX_COST;
   *x = 0;
   for (i = 0; nobetters < stop; i = (i+KSWAPINC)%degree) {
      for (r = 0; r < KSWAPSIZE-1; r++) {
         x = m.cost[r];
         mx = matrix->cost[tourcopy[(i+r)%degree]];
         for (s = 0; s < KSWAPSIZE; s++)
            x[s] = mx[tourcopy[(i+s)%degree]];
         *x = MAX_COST; /*0*/
      }
      ft = new FindTour(slow?"sl":"best", &m, &duration, 0, NULL);
      assert(ft->tour->is_complete(m.degree));
      for (p = ft->tour->get_head(); p->id != 0; p = p->get_next())
         ;
      ft->tour->change_head_to(p);
      ti.init(*ft->tour);
      found_better = 0;
      for (t = i; (p = ti.next()) != NULL; t = (t+1)%degree) {
         assert(t < degree && t != (i+KSWAPSIZE)%degree);
         if (t != (i + p->id)%degree) {
            found_better = 1;
            tourcopy[t] = xtracopy[(i+p->id)%degree];
         }
      }
      if (found_better) {
         nobetters = 0;
#ifdef PRINTITK
         sum_t before = 0, after = 0;
         for (r = 0; r < KSWAPSIZE-1; r++) {
            dump << xtracopy[(i+r)%degree]
             << "[" << matrix->val(xtracopy[(i+r)%degree],
             xtracopy[(i+r+1)%degree]) << "] ";
            before += matrix->val(xtracopy[(i+r)%degree],
             xtracopy[(i+r+1)%degree]);
            dump.flush();
         }
         dump << xtracopy[(i+r)%degree] << "\n";
         for (r = 0; r < KSWAPSIZE-1; r++) {
            dump << tourcopy[(i+r)%degree]
             << "[" << matrix->val(tourcopy[(i+r)%degree],
             tourcopy[(i+r+1)%degree]) << "] ";
            after += matrix->val(tourcopy[(i+r)%degree],
             tourcopy[(i+r+1)%degree]);
            dump.flush();
         }
         dump << tourcopy[(i+r)%degree] << " == " << (before-after) << "\n";
         dump.flush();
#endif
         for (r = 0; r < KSWAPSIZE; r++)
            xtracopy[(i+r)%degree] = tourcopy[(i+r)%degree];
      }
      else
         nobetters++;
      delete ft;
   }
}
