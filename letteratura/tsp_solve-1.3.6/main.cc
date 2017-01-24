/* main.cc
 *
 * main to trigger solvers and heuristic runs by the parameters given
 *
 * orig ceh
 */

/*
#define OPT_CHECK
 */

#include "matrix.h"
#include "standard.h"
#include "onetsolv.h"
#include "tour.h"
#include "params.h"
#include "runer.h"
#include <assert.h>
#include "chnew.h"
#include "parallel.h"

#include "binfile.h"

main(int argc, char *argv[])
{
   long shown;
   Matrix *m;

   param.init(argc, argv);
   parallel.init();
   do {
      shown = 0;
      runer.init();
      if (param.num_abrevs != 0) {
         m = new Matrix(param.degree);
         do {
            param.reseed();
            param.seed++;
            m->make_matrix(param.matrix_type, param.filename);
            if (param.verbose > 2)
               m->print();
#ifdef OPT_CHECK
            if (runer.optimal_compare(m)) {
               dump << "Optimal Compare Stopped at Seed "
                  << (param.seed-1) << "\n";
               break;
            }
#else
            runer.run(m);
#endif
         } while (runer.not_done());
         delete m;
      }
      if (parallel.my_session_done()) {
         runer.show();
         shown = 1;
      }
   } while (parallel.found_something_to_do());
   if (parallel.my_session_done() && !shown) {
      runer.show();
      shown = 1;
   }
   if (param.num_abrevs == 0)
      dump << "No Tour Finders Specified\n";
   runer.Runer::~Runer();
   parallel.destruct(1);
   dump << "Completed\n";
   param.Params::~Params();
   dump.Term::~Term();
   dumpout.Term::~Term();
   report_space(1);
   return 0;
}
