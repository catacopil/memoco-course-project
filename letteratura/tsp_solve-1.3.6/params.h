/* params.h
 * the command line argument (parameter) class
 * orig ceh
 */
#ifndef _PARAMS_H
#define _PARAMS_H

#include "stdmacro.h"
#include "binfile.h"

#define MAXABREVS 20
#define PARSPACECHAR '^'

// bits of param.recover_bits
#define RECOVERTOURFINDER 0x1
#define RECOVERRUNER 0x2
#define RECOVERMATRIX 0x4
#define RECOVERFINISHED 0x8
#define RECOVERINFO (RECOVERTOURFINDER|RECOVERRUNER|RECOVERMATRIX\
 |RECOVERFINISHED)

class Params {
   void save(BinFile &) const;
public:
   Params();
   ~Params();

   void clear();
   void try_close();
   void init(int argc, char *argv[]);
   // the argument will may be mangled or undefined after use
   void init(const char *);
   void print();
   char **new_argv;
   int new_argc;

   city_id_t degree;
   long times;
   unsigned seed;
   int printing;
   int extra; // used for miscelaneous abilities triggered by comline
   int recover_bits;
   int dist_num; // overriding distance function for euclidean tsps

   // if the current tourfinder(s) are running as invoked from
   // comand line (non-zero) or if running recursivley underneath the invoked
   // tourfinder(s) (zero).
   int invoked;

   // the default is zero and that will simply print the name of the
   // tour finder and the cost of the tour it found plus any other
   // information on a single line for each tourfinder called
   //
   // the greater verbose is past 1 the more information will be given
   int verbose;

   sum_t most;
   city_id_t initial_choice;
   int matrix_type;

   // re-seed the random generator
   void reseed();

   char *abrevs[MAXABREVS];

   // name of invoked program path
   char *progname;

   // name of directory to look for parallel files
   char *nfsdir;
   short num_abrevs;

   // Items not set by command line
   double trains;
   double explores;

   char *filename, *recover_file, *recover_ptr;

   void read(BinFile &);

   // split the work of the runner left to do in the percentage given
   void write(BinFile &, float pct) const;

   // save the whole of the parameters left and assume the tour passed
   // is the current best tour found of the single tourfinder being saved
   void write(BinFile &) const;

   BinFile *bf;
};

extern Params param;

#endif
