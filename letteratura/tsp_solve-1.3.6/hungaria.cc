/* hungaria.cc
 *
 * the Hungarian method B&B optimal solver.
 * orig Roger J. Craig
 * modified ceh from c to c++ tourfinder
 * sped up, removed comments, and made into member functions ceh 6-9-95
 */

#include "hungaria.h"
#include <assert.h>
#include "chnew.h"
#include "findtour.h"

#define MIN(I,J)  ( ((I) < (J)) ? (I) : (J) )

sum_t HungarianSolver :: bestedge(city_id_t *row, city_id_t *col,
   city_id_t size, city_id_t **r, city_id_t **c)
{
   city_id_t zeros, rowi, colj;
   cost_t mincolelt, minrowelt, ptr, *matcost;
   city_id_t *colend = col+size, *rowend = row+size, *rowip, *coljp, *kp;
   sum_t most = MIN_SUM;

   for (rowip = row; rowip < rowend; rowip++) {
      matcost = imatcost[rowi = *rowip];
      for (coljp = col; coljp < colend; coljp++) {
         colj = *coljp;
         if (matcost[colj] == 0 && rowi != colj) {
            minrowelt = MAX_COST;
            zeros = 0;
            for (kp = col; kp < colend; kp++) {
               if (rowi != *kp) {
                  ptr = matcost[*kp];
                  if (ptr == 0)
                     zeros++;
                  else
                     minrowelt = MIN(minrowelt, ptr);
               }
            }
            if (zeros > 1)
               minrowelt = 0;
            mincolelt = MAX_COST;
            zeros = 0;
            for (kp = row; kp < rowend; kp++) {
               if (*kp != colj) {
                  ptr = imatcost[*kp][colj];
                  if (ptr == 0)
                     zeros++;
                  else
                     mincolelt = MIN(mincolelt, ptr);
               }
            }
            if (zeros > 1)
               mincolelt = 0;
            if (minrowelt + mincolelt > most) {
               most = minrowelt + mincolelt;
               *r = rowip;
               *c = coljp;
            }
         }
      }
   }
   die_if(most == MIN_SUM, "Fatal Hungarian Error", "");
   return most;
}

sum_t HungarianSolver :: reduce(city_id_t size,
   city_id_t *row, city_id_t *col, cost_t *rowred, cost_t *colred)
{
   city_id_t rowi, colj;
   cost_t *ptr, temp, *matcost;
   sum_t rvalue;
   city_id_t *colend = col+size, *rowend = row+size, *rowip, *coljp;

   rvalue = 0;
   for (rowip = row; rowip < rowend; rowip++) {
      matcost = imatcost[rowi = *rowip];
      temp = MAX_COST;
      for (coljp = col; coljp < colend; coljp ++)
         if (rowi != *coljp)
            temp = MIN(temp, matcost[*coljp]);
      if (temp >= MAX_COST)
         temp = 0;
      if (temp > 0) {
         for (coljp = col; coljp < colend; coljp++) {
            if (rowi != *coljp) {
               ptr = matcost + *coljp;
               if (*ptr < MAX_COST) {
                  *ptr -= temp;
               }
            }
         }
         rvalue += temp;
      }
      rowred[rowip-row] = temp;
   }
   for (coljp = col; coljp < colend; coljp++) {
      colj = *coljp;
      temp = MAX_COST;
      for (rowip = row; rowip < rowend; rowip++)
         if (*rowip != colj)
            temp = MIN(temp, imatcost[*rowip][colj]);
      if (temp >= MAX_COST)
         temp = 0;
      if (temp > 0) {
         for (rowip = row; rowip < rowend; rowip++) {
            if (*rowip != colj) {
               ptr = imatcost[*rowip] + colj;
               if (*ptr < MAX_COST) {
                  *ptr -= temp;
               }
            }
         }
         rvalue += temp;
      }
      colred[coljp-col] = temp;
   }
   return rvalue;
}

void HungarianSolver :: explore(city_id_t size, sum_t cost,
   city_id_t *row, city_id_t *col)
{
   city_id_t first, i, last, rowi;
   city_id_t *c, *r, *kp;
   sum_t lowerbound;
   cost_t *colred, *rowred, colrowval, *matcost;
   city_id_t *newcol, *newrow;
   int avoid;
   city_id_t *colend, *rowend, *rowip, *coljp;

   colred = (rowred = new cost_t [size*2]) + size;
   cost += reduce(size, row, col, rowred, colred);
   if (cost < tweight) {
      if (size == 2) {
         for (i=0; i<degree; i++)
            best[i] = fwdptr[i];
// avoid = (imatcost[row[0]][col[0]] >= MAX_COST || col[0] == row[0]) ? 0:1;
         avoid = (imatcost[row[0]][col[0]] >= MAX_COST) ? 0:1;
         best[row[0]] = col[1-avoid];
         best[row[1]] = col[avoid];
         tweight = cost;
      }
      else {
         lowerbound = cost + bestedge(row, col, size, &r, &c);
         fwdptr[*r] = *c;
         backptr[*c] = *r;
         last = *c;
         first = *r;
         while (fwdptr[last] != NO_ID)
            last = fwdptr[last];
         while (backptr[first] != NO_ID)
            first = backptr[first];
         colrowval = imatcost[last][first];
         imatcost[last][first] = MAX_COST;
         size--;
         newcol = (newrow = new city_id_t[size*2]) + size;
         colend = col+size;
         rowend = row+size;
         for (kp = newrow, rowip = row; rowip < rowend; rowip++, kp++)
            *kp = (rowip < r) ? *rowip : rowip[1];
         for (kp = newcol, coljp = col; coljp < colend; coljp++, kp++)
            *kp = (coljp < c) ? *coljp : coljp[1];
         explore(size, cost, newrow, newcol);
         delete newrow;
         imatcost[last][first] = colrowval;
         fwdptr[*r] = backptr[*c] = NO_ID;
         size++;
         if (lowerbound < tweight) {
            imatcost[*r][*c] = MAX_COST;
            explore(size, cost, row, col);
            imatcost[*r][*c] = 0;
         }
      }
   }
   colend = col+size;
   rowend = row+size;
   for (rowip = row; rowip < rowend; rowip++) {
      matcost = imatcost[rowi = *rowip];
      for (coljp = col; coljp < colend; coljp ++) {
         if (rowi != *coljp)
            matcost[*coljp] += (cost_t)(rowred[rowip-row] + colred[coljp-col]);
      }
   }
   delete rowred;
}

int HungarianSolver :: run()
{
   city_id_t i, index;

   explore(degree, 0, major_row, major_col);
   for (i=0, index=0; i<degree; i++) {
      tour->travel(index);
      index = best[index];
   }
   return 0;
}

int HungarianSolver :: can_run(const Matrix *m) const
{
   return m!=NULL;
}

HungarianSolver :: ~HungarianSolver()
{
   delete best;
   delete wmat;
}

HungarianSolver :: HungarianSolver (const Matrix *m) : TourFinder(m)
{
   city_id_t i;
   wmat = new Matrix(*matrix);
   imatcost = wmat->cost;
   major_row = (major_col = (fwdptr = (backptr = (best
    = new city_id_t[degree*5]) + degree) + degree) + degree) + degree;
   count = 0;
   tweight = MAX_SUM;
   for (i=0; i<degree; i++) {
      major_row[i] = major_col[i] = i;
      fwdptr[i] = backptr[i] = NO_ID;
   }
}
