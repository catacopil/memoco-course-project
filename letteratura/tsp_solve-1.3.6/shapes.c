
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "rand.h"

/* maximum distance between each point
 */
#define MAX_SPACE 1000

/*
#define SHUFFLED
#define LINE
#define CIRCLE
 */
#define GRID

#ifdef CIRCLE
void circle(double SIZE)
{
   double increment = 2*M_PI/SIZE, end = 2*M_PI-increment/2, i;
   for (i = 0.0; i < end; i += increment) {
      printf("%lf %lf\n",
         (double)MAX_SPACE+1+MAX_SPACE*cos(i)/2,
         (double)MAX_SPACE+1+MAX_SPACE*sin(i)/2
      );
   }
}
#endif

#ifdef GRID
void grid(double SIZE)
{
   int side=ceil(sqrt(SIZE)), x, y, i=0;
   double dist=MAX_SPACE/(side-1), r1, r2, d=dist/10;

   for (y = 0; y < side; y++) {
      for (x = 0; x < side; x++) {
         if (++i > SIZE) {
            x = y = side;
            break;
         }
#ifdef SHUFFLED
         if (x != 0 && x != side-1 && y != 0 && y != side-1) {
            r1 = 2*d*((double)rand()/MAX_RAND)-d;
            r2 = 2*d*((double)rand()/MAX_RAND)-d;
         }
         else {
#endif
            r1 = 0.;
            r2 = 0.;
#ifdef SHUFFLED
         }
#endif
         printf("%f %f\n", (double)x*dist+r1, (double)y*dist+r2);
      }
   }
}
#endif

#ifdef HEX
void hexagon(double SIZE)
{
   int side=ceil(sqrt(SIZE)), x, y, i=0;
   double dist=MAX_SPACE/(side-1);

   for (y = 0; y < side; y++) {
      for (x = 0; x < side; x++) {
         if (++i > SIZE) {
            x = y = side;
            break;
         }
         printf("%lf %lf\n", (double)x*dist, (double)y*dist);
      }
   }
}
#endif

int main (int argc, char *argv[])
{
   double SIZE;
   srand(time(0));
   if (argc > 1)
      sscanf(argv[1], "%lf", &SIZE);
#ifdef CIRCLE
   circle(SIZE);
#endif
#ifdef GRID
   grid(SIZE);
#endif
#ifdef HEX
   hexagon(SIZE);
#endif
   return 0;
}
