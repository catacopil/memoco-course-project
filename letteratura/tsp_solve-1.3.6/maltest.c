
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "config.h"
#include "challoc.h"
#include "rand.h"

#define MNUM 10000
#define BSIZE 4
#define ESIZE 10
#define ISIZE 2
#define SPACES ((ESIZE-BSIZE)/ISIZE+1)

#define Free(_p) if ((_p) != NULL) { free(_p); (_p) = NULL; }

void straight_test(char ***space)
{
   char ***iii;
   int num, sze;

   for (num = 0; num < MNUM; num++) {
      for (sze=BSIZE-ISIZE, iii=space; iii<space+SPACES+1; iii++, sze+=ISIZE) {
         if ((*iii)[num] == NULL)
            (*iii)[num] = (char*)malloc(sze);
         else
            (*iii)[num] = (char*)realloc((*iii)[num], sze+ISIZE);
      }
   }
   /*
   report_space(1);
    */
   fprintf(stderr, "______\n");
}

void random_free(char ***space, int most)
{
   char ***iii;
   int num, i;

   for (num = 0; num < most; num++) {
      for (iii=space; iii<space+SPACES+1; iii++) {
         i = (rand()%(MNUM-num))+num;
         Free((*iii)[i]);
         (*iii)[i] = (*iii)[num];
         (*iii)[num] = NULL;
      }
   }
}

void free_all(char ***space)
{
   char ***iii, **ii;

   for (iii=space; iii<space+SPACES+1; iii++) {
      for (ii = *iii; ii < *iii+MNUM; ii++)
         Free(*ii);
      Free(*iii);
   }
}

int main ()
{
   char **space[SPACES+1], ***iii, **ii;
   int sze;

   for (sze=BSIZE-ISIZE, iii=space; iii<space+SPACES+1; iii++, sze+=ISIZE) {
      *iii = (char**)malloc(MNUM*sizeof(char*));
      for (ii = *iii; ii < *iii+MNUM; ii++)
         *ii = NULL;
      if (iii != space)
         install_blocked_mallocs(sze, MNUM/10);
   }
   assert(sze == ESIZE+ISIZE);
   straight_test(space);
   random_free(space, MNUM/2);
   straight_test(space);
   random_free(space, MNUM/2);
   straight_test(space);
   free_all(space);
   report_space(1);
   return 0;
}
