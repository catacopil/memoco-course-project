/* matrix.cc
 *
 * matrix class for travelling salesperson problems.
 *
 * orig ceh
 */

#include "matrix.h"
#include "stdmacro.h"
#include "chlib.h"
#include "params.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "rand.h"

// flags for the type of matrix
#define MAT_SYMMETRIC 0x0001
#define MAT_GEOMETRIC_2D 0x0002
#define MAT_GENERATED 0x0004
#define MAT_UNDEFINED 0x8000
#define MAT_ONEWAY 0x0008

#define MATRIX_UNIQ 0x7747418L

void Matrix :: construct(const long N)
{
   long x;

   die_if(N <= 0 || N > MAX_DEGREE, "Size of TSP out of range: ", MAX_DEGREE);
   cost = new cost_t *[(unsigned short)N];
   for (x = 0; x < N; x++)
      cost[(unsigned short)x] = new cost_t[(unsigned short)N];
   degree = (city_id_t)N;
   pos = NULL;
   type = MAT_UNDEFINED;
   dist_funct = NULL;
   incosts = NULL;
}

void Matrix :: make_incosts()
{
   city_id_t x, y;
   cost_t *co;
   if (incosts == NULL) {
      incosts = new cost_t *[degree];
      for (x = 0; x < degree; x++)
         incosts[x] = new cost_t[degree];
   }
   for (x = 0; x < degree; x++) {
      co = incosts[x];
      for (y = 0; y < degree; y++)
         co[y] = cost[y][x];
   }
}

Matrix :: Matrix(const long N)
{
   construct(N);
}

void Matrix :: operator = (const Matrix &M)
{
   city_id_t x;
   die_if(M.degree != degree, "Incompatable Matricies", "");
   dist_funct = M.dist_funct;
   for (x = 0; x < degree; x++)
      memcpy(cost[x], M.cost[x], sizeof(cost_t)*degree);
   if (M.incosts != NULL)
      make_incosts();
   if (M.pos != NULL) {
      for (x = 0; x < degree; x++)
         pos[x] = M.pos[x];
   }
   else if (pos != NULL) {
      delete pos;
      pos = NULL;
   }
   type = M.type;
}

Matrix :: Matrix (const Matrix &M)
{
   construct(M.degree);
   if (M.pos != NULL)
      pos = new pos_t[degree];
   *this = M;
}

void Matrix :: read(BinFile &t)
{
   long uniq;
   int dnum;
   city_id_t x, y;
   _uchar inc;

   t >> uniq;
   die_if(uniq != MATRIX_UNIQ, "File Reading Error", "");
   this->Matrix::~Matrix();
   t >> degree;
   construct(degree);
   t >> type >> inc >> dnum;
   dist_funct = find_dname_from_num(dnum)->dist;
   if (type & MAT_GEOMETRIC_2D) {
      pos = new pos_t[degree];
      for (x = 0; x < degree; x++)
         t >> pos[x].x >> pos[x].y;
      plot_planar(dist_funct);
   }
   else  {
      for (x = 0; x < degree; x++) {
         for (y = 0; y < degree; y++)
            t >> cost[x][y];
      }
   }
   if (inc)
      make_incosts();
#ifndef OLD_TIMER
   t >> uniq;
   die_if(uniq != MATRIX_UNIQ, "File Reading Error", "");
#endif
}

void Matrix :: write(BinFile &t) const
{
   city_id_t x, y;
   _uchar inc = (_uchar)(incosts != NULL);

   t << MATRIX_UNIQ << degree << type << inc
      << find_dname_from_dist(dist_funct)->num;
   if (type & MAT_GEOMETRIC_2D) {
      assert(pos != NULL);
      for (x = 0; x < degree; x++)
         t << pos[x].x << pos[x].y;
   }
   else {
      for (x = 0; x < degree; x++)
         for (y = 0; y < degree; y++)
            t << cost[x][y];
   }
#ifndef OLD_TIMER
   t << MATRIX_UNIQ;
#endif
}

Matrix :: ~Matrix()
{
   city_id_t x;
   if (incosts != NULL) {
      for (x = 0; x < degree; x++)
         delete incosts[x];
      delete incosts;
   }
   for (x = 0; x < degree; x++)
      delete cost[x];
   delete cost;
   if (pos != NULL)
      delete pos;
}

void Matrix :: plot_planar()
{
   plot_planar(default_distance);
}

void Matrix :: plot_planar(dist_f dist)
{
   city_id_t x,y;
   cost_t *ycost;
   pos_t *site_pos = pos;

   assert(dist != NULL);
   if (param.dist_num == -1)
      dist_funct = dist;
   else
      dist_funct = find_dname_from_num(param.dist_num)->dist;
   for (y = 0; y < degree; y++) {
      ycost = cost[y];
      for (x = (city_id_t)(y+1); x < degree; x++) {
         ycost[x] = cost[x][y]
            = (cost_t)dist_funct(site_pos+x, site_pos+y);
         die_if(!(ycost[x] < COST_MAX && ycost[x] >= 0), "Bad Cost", ycost[x]);
      }
      cost[y][y] = COST_MAX;
   }
   type = MAT_GEOMETRIC_2D|MAT_SYMMETRIC;
}

void Matrix :: planar()
{
   city_id_t y;

   if (pos == NULL)
      pos = new pos_t[degree];
   for (y = 0; y < degree; y++) {
      pos[y].x = (MAX_POS * rand())/ MAX_RAND;
      pos[y].y = (MAX_POS * rand())/ MAX_RAND;
   }
   plot_planar();
   type |= MAT_GENERATED;
}

#define RAND(max) (((unsigned long)rand()/(double)MAX_RAND)*(max))
void Matrix :: internal_make(long max, long max_diff)
{
   city_id_t y, x;

   die_if((cost_t)(max+max_diff) >= COST_MAX || max_diff < -1, "Bad Param", "");
   if (pos != NULL) {
      delete pos;
      pos = NULL;
   }
   for (y = 0; y < degree; y++) {
      for (x = 0; x < degree; x++) {
         switch(max_diff) {
         default:
            if (x > y)
               cost[y][x] = (cost_t)(RAND(max-max_diff)+max_diff);
            else
               cost[y][x] = (cost_t)(cost[x][y]+(RAND(2*max_diff))-max_diff);
            break;
         case 0:
            if (x > y)
               cost[y][x] = (cost_t)(RAND(max));
            else
               cost[y][x] = cost[x][y];
            break;
         case -1:
            cost[y][x] = (cost_t)(RAND(max));
            break;
         }
      }
      cost[y][y] = COST_MAX;
   }
}

void Matrix :: symmetric()
{
   internal_make((int)(MAX_COST/2), 0);
   type = MAT_GENERATED|MAT_SYMMETRIC;
}

void Matrix :: knight()
{
   city_id_t x, y;
   if (pos == NULL)
      pos = new pos_t[degree];
   pos[0].x = 0; pos[0].y = 0;
   for (x = 1; x < degree; x++) {
		/*
      pos[x].x = rand()%101;
      pos[x].y = rand()%101;
		 */
		/*
		pos[x].x = pos[x-1].x + rand()%61 - 30;
		pos[x].y = pos[x-1].y + rand()%61 - 30;
		 */
		pos[x].x = 50+(int)((50*(degree-5*x/6)/degree)*cos(x*(6.*M_PI/degree))) + 2 -(rand()%5);
		pos[x].y = 50+(int)((50*(degree-5*x/6)/degree)*sin(x*(6.*M_PI/degree))) + 2 -(rand()%5);
		/*
		pos[x].x = pos[x-1].x + 1 + rand()%10;
		if (pos[x].x > 100) {
			pos[x].y = (int)(pos[x-1].y + (pos[x].x/100))%101;
			pos[x].x = (int)pos[x].x%101;
		}
		else
			pos[x].y = pos[x-1].y;
			 */
		y=0;
		if ((pos[x].x >= 0 && pos[x].y >= 0) && pos[x].x < 101 && pos[x].y < 101)
	      for (; y < x; y++)
	         if (pos[y].x == pos[x].x && pos[y].y == pos[x].y)
	            break;
      if (y < x) {
		/*
		dump << pos[x];
		 */
         x--;
         continue;
      }
   }
   dist_funct = knight_distance;
   plot_planar();
   type |= MAT_GENERATED;
}

void Matrix :: oneway()
{
   city_id_t x;
   degree -= (city_id_t)2;
   internal_make(9700, 300);
   for (x = 0; x < degree; x++) {
      cost[degree][x] = (cost_t)(rand()%10000);
      cost[x][degree] = COST_MAX;
   }
   for (x = 0; x < degree; x++) {
      cost[x][degree+1] = (cost_t)(rand()%10000);
      cost[degree+1][x] = COST_MAX;
   }
   cost[degree+1][degree+1] = cost[degree][degree] = cost[degree][degree+1] =
      COST_MAX;
   cost[degree+1][degree] = 0;
   degree += (city_id_t)2;
   type = MAT_GENERATED|MAT_ONEWAY;
}

void Matrix :: asymmetric()
{
   internal_make(10000, -1);
   type = MAT_GENERATED;
}

void Matrix :: user_input_planar_order(Term &file)
{
   city_id_t i, index, x, base0 = 1;
   pos_t *temp_pos = new pos_t[degree];
   city_id_t *temp_ord = new city_id_t[degree];
   double in;

   if (pos == NULL)
      pos = new pos_t[degree];
   for (i = 0; i < degree; i++) {
      file >> temp_pos[i].x >> temp_pos[i].y;
      pos[i].x = MAX_FLOAT;
   }
   for (i = 0; i < degree && !file.eof(); i++) {
      file >> in;
      x = (city_id_t)(in+.5);
      if (base0) {
         if (x == degree) {
            base0 = 0;
            for (index = 0; index < i; index++) {
               die_if(temp_ord[index] == 0 , "Tour Reading Erorr", "");
               temp_ord[index]--;
            }
            x--;
         }
         else
#ifdef UNSIGNED_CITY_ID
            die_if(x >= degree, "City Id out of range", x);
#else
            die_if(x >= degree || x < 0, "City Id out of range", x);
#endif
         temp_ord[i] = x;
      }
      else {
         die_if(x > degree || x <= 0, "City Id out of range", x);
         temp_ord[i] = x-1;
      }
   }
   die_if(i < degree, "End Of File Error", "");
   for (i = 0; i < degree; i++) {
      pos[i] = temp_pos[temp_ord[i]];
      if (param.verbose) {
         if (!((i+1) % 15))
            dumpout << "\n";
         dumpout << temp_ord[i] << ", ";
      }
   }
   if (param.verbose)
      dumpout << "\n";
   for (i = 0; i < degree; i++)
      die_if(pos[i].x == MAX_FLOAT, "Ordered Tour is Not complete: ", i);
   dump << "\n";
   delete temp_pos;
   delete temp_ord;
   plot_planar();
}

/* Point *1*: read an extra "end point" coordinate (two numbers)
 */
void Matrix :: user_input_planar(Term &file, int extra)
{
   city_id_t i;

   if (pos == NULL)
      pos = new pos_t[degree];
   for (i = 0; i < degree && !file.eof(); i++) {
      file >> pos[i].x >> pos[i].y;
   }
   if (extra) { /*1*/
      double in;
      file >> in;
      if (file.eof())
         i = 0;
      file >> in;
   }
   die_if(i < degree, "End Of File Error", "");
   plot_planar();
}

void Matrix :: user_input(Term &file)
{
   user_input(0, file);
}

static short get_input_format(char *f)
{
   short ret_val;
   if (strstr(f, "FULL_MATRIX"))
      ret_val = 0;
   else if (strstr(f, "LOWER_DIAG_ROW"))
      ret_val = 1;
   else if (strstr(f, "UPPER_ROW"))
      ret_val = 4;
   else {
      ret_val = 0;
      die_if(1, "Don't Know Matrix Input Format type", f);
   }
   return ret_val;
}

#define Cj (cost_t)j

void Matrix :: user_input(const short format, Term &file)
{
   city_id_t x,y;
   cost_t *ycost;
   double j;

   if (pos != NULL) {
      delete pos;
      pos = NULL;
   }
   type = MAT_SYMMETRIC;
   for (y = 0; y < degree; y++) {
      ycost = cost[y];
      for (x = 0; x < degree; x++) {
         switch(format) {
         case 0: file >> j; ycost[x] = Cj;  break;
         case 1: if (x <= y) { file >> j; cost[x][y] = ycost[x] = Cj; } break;
         case 2: if (x >= y) { file >> j; cost[x][y] = ycost[x] = Cj; } break;
         case 3: if (x < y) { file >> j; cost[x][y] = ycost[x] = Cj; } break;
         case 4: if (x > y) { file >> j; cost[x][y] = ycost[x] = Cj; } break;
         }
         if ((type & MAT_SYMMETRIC) && y > x && ycost[x] != cost[x][y])
            type &= ~(MAT_SYMMETRIC);
         die_if(ycost[x] >= COST_MAX, "Bad Cost", ycost[x]);
      }
      ycost[y] = COST_MAX;
   }
   dist_funct = NULL;
}

void Matrix :: make_type()
{
   cost_t *ycost;
   city_id_t x, y;

   type = MAT_SYMMETRIC | ((pos == NULL) ? 0 : MAT_GEOMETRIC_2D);
   for (y = 0; y < degree; y++) {
      ycost = cost[y];
      for (x = 0; x < degree; x++) {
         if ((type & MAT_SYMMETRIC) && y > x && ycost[x] != cost[x][y])
            type &= ~(MAT_SYMMETRIC);
      }
   }
}

#define CWIDTH 3
#ifndef HAS_SPRINTF_DECL
extern "C" int sprintf(char *, const char *, ...);
#endif
void Matrix :: print() const
{
   city_id_t x, y, deg = degree;
   cost_t *ycost;
   char integer[5], str[8];

   sprintf(integer, "%%%dd", CWIDTH);
   sprintf(str, "%%%d.%ds", CWIDTH, CWIDTH);
   /*
   if (is_oneway()) {
      deg -= (city_id_t)2;
      dump << deg << "\n";
      for (x = 0; x < deg; x++)
         dump.form(integer, (int)cost[deg][x]);
      dump << "\n";
      for (y = 0; y < deg; y++)
         dump.form(integer, (int)cost[y][deg+1]);
      dump << "\n";
   }
    */
   for (y = 0; y < deg; y++) {
      ycost = cost[y];
      for (x = 0; x < deg; x++) {
         if (ycost[x]==MAX_COST)
            dump.form(str, "00");
         else
            dump.form(integer, (int)ycost[x]);
      }
      dump << "\n";
   }
}

/*                   _________________________
 * Std Deviation = \/ Sigma(Xi^2)/N - (Mean)^2
 */
void Matrix :: histogram()
{
   city_id_t x,y;
   cost_t *ycost, low;
   double *stdev = new double[degree], *mean = new double[degree];
   sum_t sum, sum_sqr;

   for (y = 0; y < degree; y++) {
      ycost = cost[y];
      sum = sum_sqr = 0;
      low = COST_MAX;
      for (x = 0; x < degree; x++) {
         if (x != y) {
            if (low > ycost[x])
               low = ycost[x];
            sum += ycost[x];
            sum_sqr += ycost[x]*ycost[x];
         }
      }
      mean[y] = (double)sum/((double)degree-1);
      stdev[y] = sqrt((double)sum_sqr/((double)degree-1) - mean[y]*mean[y]);
      dump << "City " << y << " : M-D+L " << (mean[y]-stdev[y]+low) << "\n";
   }
   delete mean;
   delete stdev;
}

int Matrix :: is_symmetric() const
{
   return !(type & MAT_UNDEFINED) && (type & MAT_SYMMETRIC);
}

int Matrix :: is_geometric_2d() const
{
   return !(type & MAT_UNDEFINED) && (type & MAT_GEOMETRIC_2D);
}

int Matrix :: is_generated() const
{
   return !(type & MAT_UNDEFINED) && (type & MAT_GENERATED);
}

int Matrix :: is_oneway() const
{
   return !(type & MAT_UNDEFINED) && (type & MAT_ONEWAY);
}

void Matrix :: make_matrix(int matrix_type, const char *filename)
{
   if (param.recover_bits & RECOVERMATRIX) {
      read(*param.bf);
      param.recover_bits &= ~RECOVERMATRIX;
dump << "READMATRIX\n";
      param.try_close();
   }
   else {
      Term file(0), *fptr;
      if (filename != NULL) {
         file.open(filename, "r", 0);
         fptr = &file;
      }
      else
         fptr = &dump;
      switch(matrix_type) {
      case -6: user_input_planar(*fptr, 1); break;
      case -2: user_input_planar_order(*fptr); break;
      case -1: user_input_planar(*fptr, 0); break;
      case 0: user_input(*fptr); break;
      case 1: planar(); break;
      case 2: symmetric(); break;
      case 3: asymmetric(); break;
      case 4: oneway(); break;
      case 5: knight(); break;
      case -4: user_input_tsplib(*fptr); break;
      case -3: user_input_oneway(*fptr); break;
      case -5: user_input_oneway_symmetric(*fptr); break;
      default: die_if(1, "Undefined Matrix type", matrix_type);
      }
      if (fptr == &file)
         file.close(filename);
   }
}

void Matrix :: user_input_oneway(Term &file)
{
   double d;
   long dg;
   city_id_t deg, x;

   this->Matrix::~Matrix();
   file >> d;
   dg = (long)d;
   deg = (city_id_t)dg;
   construct(dg+2);
   for(x = 0; x < deg; x++) {
      file >> d;
      cost[deg][x] = (cost_t)d;
      cost[x][deg] = COST_MAX;
   }
   for(x = 0; x < deg; x++) {
      file >> d;
      cost[deg+1][x] = COST_MAX;
      cost[x][deg+1] = (cost_t)d;
   }
   cost[deg+1][deg+1] = cost[deg][deg] = cost[deg][deg+1] = COST_MAX;
   cost[deg+1][deg] = 0;
   degree -= (city_id_t)2;
   user_input(0, file);
   degree += (city_id_t)2;
   type &= ~(MAT_SYMMETRIC);
   type |= MAT_ONEWAY;
}

void Matrix :: user_input_oneway_symmetric(Term &file)
{
   double d;
   long dg;
   city_id_t deg, x;

   this->Matrix::~Matrix();
   file >> d;
   dg = (long)d;
   deg = (city_id_t)dg;
   construct(dg+2);
   for(x = 0; x < deg; x++) {
      file >> d;
      cost[x][deg] = cost[deg][x] = (cost_t)d;
   }
   for(x = 0; x < deg; x++) {
      file >> d;
      cost[deg+1][x] = cost[x][deg+1] = (cost_t)d;
   }
   cost[deg+1][deg+1] = cost[deg][deg] = COST_MAX;
   cost[deg][deg+1] = cost[deg+1][deg] = 0;
   degree -= (city_id_t)2;
   user_input(0, file);
   degree += (city_id_t)2;
}

#define MAXBUF 100

void Matrix :: user_input_tsplib(Term &file)
{
   city_id_t x, y, *tour=NULL;
   char buf[MAXBUF];
   double X, Y, ID;
   int matrix_done = 0, tour_done = 0;
   dist_f distance_funct = NULL;
   short input_format = 0; 

   this->Matrix::~Matrix();

   degree = 0;
   while (file.tfgets(buf, MAXBUF)) {
      if (strstr(buf, "DIMENSION") && degree == 0) {
         _uchar *str;
         for (str = (_uchar*)buf; *str != '\0' && *str != ':'; str++)
            ;
         if (*str == ':')
            str++;
         construct(atoi((char*)str));
      }
      else if (strstr(buf, "EDGE_WEIGHT_TYPE")) {
         _uchar *str;
         dname_type *dn;
         for (str = (_uchar*)buf; *str != '\0' && *str != ':'; str++)
            ;
         dn = find_dname_from_name((char*)str);
         distance_funct = dn->dist;
      }
      else if (strstr(buf, "EDGE_WEIGHT_FORMAT")) {
         _uchar *str;
         for (str = (_uchar*)buf; *str != '\0' && *str != ':'; str++)
            ;
         input_format = get_input_format((char*)str);
      }
      else if (strstr(buf, "EDGE_WEIGHT_SECTION") && degree != 0) {
         user_input(input_format, file);
         matrix_done = 1;
      }
      else if (((strstr(buf, "NODE_COORD_SECTION") && !matrix_done) ||
       (strstr(buf, "DISPLAY_DATA_SECTION") && !matrix_done && !tour_done))
       && degree != 0) {
         if (pos == NULL)
            pos = new pos_t[degree];
         for (x = 0; x < degree; x++)
            pos[x].x = pos[x].y = (double)MAX_FLOAT;
         for (x = 0; x < degree; x++) {
            file >> ID >> X >> Y;
            y = (city_id_t)ID;
            die_if(y < 1 || y > degree, "ERROR: NODE_COORD_SECTION, bad id", y);
            pos[y-1].x = X;
            pos[y-1].y = Y;
         }
         for (x = 0; x < degree; x++) {
            die_if(pos[x].x == MAX_FLOAT && pos[x].y == MAX_FLOAT,
               "No Coordinates for city id", x);
         }
         die_if(distance_funct == NULL,
            "EDGE_WEIGHT_TYPE not defined before NODE_COORD_SECTION\n", "");
         plot_planar(distance_funct);
         matrix_done = 1;
         tour_done = 1;
      }
      else if (strstr(buf, "TOUR_SECTION") && degree != 0) {
         if (tour == NULL)
            tour = new city_id_t[degree];
         for (x = 0; x < degree; x++) {
            file >> ID;
            tour[x] = (city_id_t)(ID-.5);
         }
      }
   }
   die_if(degree == 0, "No DIMENSION Given for TSPLIB File\n", "");
   die_if(!matrix_done, "TSP or coord list not specified in TSPLIB File\n", "");
   if (tour != NULL) {
      Matrix copy(*this);
      cost_t *c, *ct;
      for (x = 0; x < degree; x++) {
         if (pos != NULL && copy.pos != NULL)
            pos[x] = copy.pos[tour[x]];
         c = cost[x];
         ct = copy.cost[tour[x]];
         for (y = 0; y < degree; y++) {
            c[y] = ct[tour[y]];
         }
      }
      delete tour;
   }
}
