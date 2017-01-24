/* smatrix.h
 * the sorted matrix used with 1-trees
 * orig ceh
 */

#ifndef _SMATRIX_H
#define _SMATRIX_H

#include "stdmacro.h"
#include "matrix.h"
#include "binfile.h"

/* I may want to look at how the lambdas are created so that can use
 * floating point, but lambdas and lambda sums must be te same as costs
 * or a tour is not garuanteed to be optiomal or found.
 */
#define lambda_cost_t float
#define lambda_t double
#define lambda_sum_t double
#define MAX_LCOST ((lambda_cost_t)1e30)
/*
#define lambda_t cost_t
#define lambda_sum_t sum_t
#define lambda_cost_t cost_t
#define MAX_LCOST MAX_COST
 */

typedef struct SortedCost {
   lambda_cost_t cost;
   cost_t real_cost;
   city_id_t id;
} SortedCost;

#define smid_t unsigned long

typedef struct Exclusion {
   SortedCost *last_pos, *now_pos, sorted_cost;
   city_id_t x;
   smid_t smid;
   struct Exclusion *next;
} Exclusion;

class SortedMatrix {
   void construct(city_id_t deg);
   short status; // if status is nonzero then sort(Matrix*) was called
   smid_t smid; // last forbided smid
   Exclusion *exclusions;
   inline void resort(const city_id_t i, const lambda_t *);

public:
   city_id_t degree;
   SortedCost **cost;
   lambda_t *lambdas;
   const Matrix *matrix;

   // initializes a matrix of NxN
   SortedMatrix(city_id_t deg);
   SortedMatrix(const SortedMatrix &);
   SortedMatrix(const Matrix *, city_id_t j);
   ~SortedMatrix();

   inline cost_t lambda_val(const city_id_t a, const city_id_t b)
   {
      return (cost_t)((lambda_t)matrix->val(a,b) + lambdas[a] + lambdas[b]);
   }

   void operator=(const SortedMatrix &);
   void sort(const Matrix *, const lambda_t *);
   void resort(const city_id_t j);

   // sort the matrix by the value of j
   // j == NO_ID, sort by sc[xy] = c[xy];
   // j == MAX_DEGREE, sort by sc[xy] = c[yx];
   // otherwise sort by sc[xy] = c[xy] - c[jx] - c[yj];
   void sort(const Matrix *, city_id_t j);

   // allocate a new smid for use in backing up and restoring the
   // exclusion state of a matrix
   smid_t new_smid();

   // using the smid returned by new_smid, you may forbid an edge from
   // the matrix.
   void forbid_edge(smid_t smid, city_id_t x, city_id_t y);

   // restore to the last backed up state of smid
   // this will clear all forbids that were forbided under smid's > 'smid'
   void restore(smid_t smid);

   // functions to save and restore matrixes
   SortedMatrix(BinFile &, const Matrix *);
   void write(BinFile &) const;

   void print();
};

// start using new matrix, allocates a new matrix from the reference
// initializes use of that matrix and returns it
SortedMatrix *start_using_matrix(const SortedMatrix &sm);

void use_matrix(SortedMatrix *sm);

// like pop but only takes off if last user
void unuse_matrix(SortedMatrix *sm);

// saves the matrix stack to the BinFile
void write_matrix_stack(BinFile &);

// restores the matrix stack from the BinFile
void read_matrix_stack(BinFile &, const Matrix *);

// after a restore, there are unresolved void SortedMatrix pointers that
// can be found in the current heap by this procedurs
SortedMatrix *find_SortedMatrix_ptr(const void *);

// makes the matrix a singlely used matrix.  if the matrix is only used
// by one user then nothing is done, but if the matrix is used by another
// then the matrix is restarted
void make_lone_matrix(SortedMatrix **sm);

// starts a new matrix stack, this must be followed by the same number of
// free_matrix_stack() calls.
void init_matrix_stack(const city_id_t deg);

// free_matrix_stack() frees the current matrix stack.  It will report if
// there are any matrices left on the current stacks, and then revert to
// the last inited_matrix_stack.
void free_matrix_stack();

#endif
