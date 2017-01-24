/* matrix.h
 * the matrix class
 * orig ceh
 */

#ifndef _MATRIX_H
#define _MATRIX_H

#include "stdmacro.h"
#include "chlib.h"
#include "binfile.h"

// MAX_DEGREE is a value of the maximum number of cities possible to specify
// a TSP.  a city_id may not be either NO_ID nor MAX_DEGREE.
//
#ifdef LARGE_CITY_ID
/*
# define MAX_DEGREE 6000 // not too large here since static arrays
# define MAX_DEGREE 18520 // not too large here since static arrays
 */
# define MAX_DEGREE 2400 // not too large here since static arrays
#else
# ifdef UNSIGNED_CITY_ID
#  define MAX_DEGREE 254 // 255 would be the same as NO_ID
# else
#  define MAX_DEGREE 127 // NO_ID is still -1, so 127 is fine, 128 could work
# endif
#endif
#if (MAX_DEGREE == NO_ID)
cant_have_MAX_DEGREE_equal_to_NO_ID
#endif

class Matrix {
   void plot_planar();
   void plot_planar(dist_f);
   short type;
   void construct(const long N);
   void operator = (const Matrix &);
   void internal_make(long max, long max_diff);
public:
   city_id_t degree;
   cost_t **cost;     // list of pointers to outcosts from each city
   pos_t *pos;
   dist_f dist_funct;

   // made the parameter long because we want to assert the value is less than
   // max degree and if the value is larger than the data type we'll never
   // know, so we make the data type as large as we think we'll get input for.
   Matrix(const long N);

   Matrix(const Matrix &);
   ~Matrix();

   // consturcts the incosts array
   void make_incosts();
   cost_t **incosts;  // incosts to each city, only made upon request

   // return the cost to go from "from" to "to"
   inline cost_t val (const city_id_t from, const city_id_t to) const {
      return cost[from][to];
   };

   // assign the costs of this matrix to a certain type of generated matrix
   void planar();
   void asymmetric();
   void symmetric();
   void oneway();
   void knight();

   // initializes the map from user input
   // assumes the user knows how many costs to input
   void user_input(Term &);
   void user_input(const short, Term &);

   // takes coordinates from user
   void user_input_planar(Term &, int extra);

   // takes coordinates from user in the order that follows by N city id's
   void user_input_planar_order(Term &);

   void user_input_oneway(Term &);
   void user_input_oneway_symmetric(Term &);

   // expects input of "tsplib" format files.  This function may overwrite
   // the default degree of the matrix
   void user_input_tsplib(Term &);

   void print() const;

   // does a histogram and outputs to dump
   void histogram();

   // questions for the matrix to answer
   int is_symmetric() const;
   int is_geometric_2d() const;
   int is_generated() const;
   int is_oneway() const;

   // make_type is to be called for users that hand code the matrix and
   // make_type determines what kind of matrix it is
   void make_type();

   // makes a matrix of a documented matrix type
   // -6 is same as -1 but ignoes additional coordinate at end
   // -5 is inputing oneway symmetric
   // -4 is inputing tsplib matrix
   // -3 is inputing a one way matrix
   // -2 is user input coordinate set and ordering city id's
   // -1 is user input coordinate set making a geometric 2d matrix
   // 0 is user input row by row matrix
   // 1 is randomly generated geometric 2d matrix
   // 2 is randomly generated symmetric matrix
   // 3 is randomly generated asymmetric matrix
   // 4 is randomly generated oneway matrix
   // 5 is randomly generated knight matrix
   void make_matrix(int matrix_type, const char *filename);

   // save and restore
   void write(BinFile &) const;
   void read(BinFile &);
};

#endif
