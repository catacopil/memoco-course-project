/* params.cc
 *
 * command line argument class to accept input from a user command line
 *
 * orig ceh
 */

#include <ctype.h>
#include <stdlib.h>
#include "io.h"
#include <time.h>
#include <assert.h>
#include "params.h"
#include "chlib.h"
#include "parallel.h"
#include "rand.h"

#define DEFAULT_DEGREE 5
#define DEFAULT_INT 999
#define DEFAULT_TIMES 1

Params param;

Params :: Params()
{
   most = MAX_SUM;
   invoked = 1;
   nfsdir = NULL;
   recover_file = NULL;
   recover_ptr = NULL;
   recover_bits = 0;
   progname = NULL;
   new_argv = NULL;
   bf = NULL;
}

void Params :: try_close()
{
   if (!(recover_bits & RECOVERINFO)) {
      bf->close();
      recover_bits |= RECOVERFINISHED;
   }
}

Params :: ~Params()
{
   int x;
   if (filename != NULL) {
      delete filename;
      filename = NULL;
   }
   if (recover_ptr != NULL) {
      delete recover_ptr;
      recover_ptr = NULL;
   }
   if (recover_file != NULL) {
      if ((recover_bits & (RECOVERFINISHED)) && nfsdir != NULL) {
         dump << "*Deleteing " << recover_file << "\n";
         parallel.remove_tmpname(recover_file);
         recover_bits = 0;
      }
      delete recover_file;
      recover_file = NULL;
   }
   if (nfsdir != NULL) {
      delete nfsdir;
      nfsdir = NULL;
   }
   if (new_argv != NULL) {
      for (new_argc--; new_argc>0; new_argc--)
         delete new_argv[new_argc];
      delete new_argv;
      new_argv = NULL;
   }
   if (bf != NULL) {
      delete bf;
      bf = NULL;
   }
   for (x = 0; x < num_abrevs; x++) {
      if (abrevs[x] != NULL) {
         delete abrevs[x];
         abrevs[x] = NULL;
      }
   }
   num_abrevs = 0;
}

void Params :: reseed()
{
   srand(seed);
   /*
   int x;
   x = rand();
    */
}

void Params :: clear()
{
   char *argv[]={progname, NULL};
   assert(progname != NULL);
   init(1, argv);
}

// this does change "args" but should be garuanteed to change it back
//
// FIX!!! this does not need to change args, you can make a copy and then
// zero out the PARSPACEs, and assign the arg pointers to the addresses in
// a large string, and not have to have so many little strings.
void Params :: init(const char *args)
{
   int under;
   char *newarg, *argu = (char*)args;

   if (new_argv != NULL) {
      for (new_argc--; new_argc>0; new_argc--)
         delete new_argv[new_argc];
      delete new_argv;
   }
   for (under = 1, newarg = argu; *newarg != 0; newarg++) {
      if (*newarg == PARSPACECHAR)
         under++;
   }
   new_argv = new char*[under+1];
   assert(progname != NULL);
   *new_argv = progname;
   for (new_argc = 1; *argu != NULLCHAR && *argu != 0; new_argc++) {
      for (newarg = argu; *newarg != PARSPACECHAR && *newarg != 0; newarg++)
         ;
      if (*newarg == PARSPACECHAR) {
         *newarg = 0;
         under = 1;
      }
      else
         under = 0;
      new_argv[new_argc] = strduup(argu, 0);
      if (under) {
         argu = newarg + 1;
         *newarg = PARSPACECHAR;
      }
      else
         argu = newarg;
   }
   init(new_argc, new_argv);
}

void Params :: init(int argc, char *argv[])
{
   int arg, i;
   _uchar c;
   _uchar **arguv = (_uchar**)&(argv[0]); // to keep from promoting unsigned

   progname = *argv;
   num_abrevs = 0;
   seed = (unsigned)time(0);
   times = DEFAULT_TIMES;
   degree = NO_ID;
   initial_choice = NO_ID;
   verbose = matrix_type = DEFAULT_INT;
   extra = printing = 0;
   dist_num = -1;
   filename = NULL;
   for (arg = 1; arg < argc; arg++) {
      if (*arguv[arg] == '-' && arguv[arg][1] == 'n') {
#ifdef HAS_NFSLOCK
         i = strlen(argv[arg]+2);
         assert(i > 0);
         if (nfsdir != NULL) {
            dump << "More than one nfsdir: using '" << argv[arg]+2 << "'.\n";
            delete nfsdir;
         }
         nfsdir = strduup(argv[arg]+2, 1);
         if (nfsdir[i-1] != '/') {
            nfsdir[i] = '/';
            nfsdir[i+1] = 0;
         }
#else
         dump << "This Machine does not support parallel mode.\n";
#endif
      }
   }
   for (arg = 1; arg < argc; arg++) {
      if (*arguv[arg] == '-') switch (c = arguv[arg][1]) {
      case 'i': initial_choice = (city_id_t)atoi(argv[arg]+2); break;
      case 's': seed = atoi(argv[arg]+2); break;
      case 't': times = atoi(argv[arg]+2); break;
      case 'o': most = (sum_t)atof(argv[arg]+2); break;
      case 'p': printing = atoi(argv[arg]+2); break;
      case 'x': extra = atoi(argv[arg]+2); break;
      case 'r':
         if (recover_ptr != NULL)
            delete recover_ptr;
         recover_ptr = strduup(argv[arg]+2, 0);
         if (*recover_ptr == 0) {
            delete recover_ptr;
            recover_ptr = strduup("state000.tsp", 0);
         }
         if (recover_file != NULL) {
            if ((recover_bits & (RECOVERFINISHED)) && nfsdir != NULL) {
               dump << "*Deleteing " << recover_file << "\n";
               parallel.remove_tmpname(recover_file);
               recover_bits = 0;
            }
            delete recover_file;
            recover_file = NULL;
         }
         if (nfsdir != NULL) {
            char buf[200];
            sprintf(buf, "%s%s", nfsdir, recover_ptr);
            recover_file = strduup(buf, 0);
         }
         else
            recover_file = strduup(recover_ptr, 0);
         break;
      case 'd': dist_num = find_dname_from_name(argv[arg]+2)->num; break;
      case 'v': if ((verbose = atoi(argv[arg]+2)) == 0) verbose = 1; break;
      case 'f': filename = strduup(argv[arg]+2, 0); break;
      default:
         if (c=='m') {
            argv[arg]++;
            c = argv[arg][1];
         }
         if (isdigit(c) || c == 0 || c == '=' || c == '-') {
            if (matrix_type != DEFAULT_INT)
               dump << "More than one form of matrix: using form '"
                  << atoi(argv[arg]+1) << "'.\n";
            if (c == '=')
               matrix_type = -2;
            else if (c == '-' && arguv[arg][2] == 0)
               matrix_type = -4;
            else
               matrix_type = atoi(argv[arg]+1);
         }
         if (arguv[arg][0] == 'm')
            argv[arg]--;
         break;
      }
      else if ((i = atoi(argv[arg])) != 0) {
         if (degree != NO_ID)
            dump << "More than one degree specified: using " << i << ".\n";
         degree = (city_id_t)i;
      }
      else if (*arguv[arg] == '=') {
         if (matrix_type != DEFAULT_INT)
            dump << "More than one form of matrix: using form =\n";
         matrix_type = ((arguv[arg][1] == '.') ? -6 : -1);
      }
      else {
         assert(num_abrevs < MAXABREVS);
         abrevs[num_abrevs++] = strduup(argv[arg], 0);
      }
   }
   if (matrix_type == DEFAULT_INT)
      matrix_type = 1;
   if (degree == NO_ID)
      degree = DEFAULT_DEGREE;
   if (verbose == DEFAULT_INT)
      verbose = 0;
   if (recover_ptr != NULL) {
      if (bf != NULL)
         delete bf;
      bf = new BinFile(recover_file, "rb", 0);
#ifdef OLD_TIMER
      recover_bits |= RECOVERTOURFINDER;
#else
      this->read(*bf);
#endif
   }
   print();
   reseed();
}

void Params :: print()
{
   int x;
   dump << "-s" << seed;
   if (matrix_type != 1)
      dump << " -m" << matrix_type;
   if (degree != DEFAULT_DEGREE)
      dump << " " << degree;
   if (times != DEFAULT_TIMES)
      dump << " -t" << times;
   if (initial_choice != NO_ID)
      dump << " -i" << initial_choice;
   if (dist_num != -1)
      dump << " -d" << find_dname_from_num(dist_num)->name;
   if (most != MAX_SUM)
      dump << " -o" << most;
   if (verbose != 0)
      dump << " -v" << verbose;
   if (printing != 0)
      dump << " -p" << printing;
   if (recover_ptr != NULL)
      dump << " -r" << ((*recover_ptr) ? recover_ptr : " ");
   if (extra != 0)
      dump << " -x" << extra;
   if (nfsdir != NULL)
      dump << " -n" << nfsdir;
   if (filename != NULL)
      dump << " -f" << filename;
   for (x = 0; x < num_abrevs; x++)
      dump << " " << abrevs[x];
   dump << "\n";
}

void Params :: save(BinFile &file) const
{
   int x;

   file << seed;
   file << matrix_type;
   file << degree;
   file << times;
   file << initial_choice;
   file << dist_num;
   file << most;
   file << verbose;
   file << printing;
   file << recover_bits;
   file << extra;
   file << filename;
   file << num_abrevs;
   for (x = 0; x < num_abrevs; x++)
      file << abrevs[x];
}

void Params :: write(BinFile &file, float /*pct*/) const
{
   save(file);
}

void Params :: write(BinFile &file) const
{
   save(file);
}

void Params :: read(BinFile &file)
{
   int x;

   file >> seed;
   file >> matrix_type;
   file >> degree;
   file >> times;
   file >> initial_choice;
   file >> dist_num;
   file >> most;
   file >> verbose;
   file >> printing;
   file >> recover_bits;
   recover_bits &= ~(RECOVERFINISHED);
   file >> extra;
   file >> filename;
   file >> num_abrevs;
   for (x = 0; x < num_abrevs; x++)
      file >> abrevs[x];
}
