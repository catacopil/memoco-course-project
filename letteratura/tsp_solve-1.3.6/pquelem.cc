/* pquelem.cc
 *
 * Priority Queue element base
 *
 * orig ceh
 */

#include "chnew.h"
#include "pquelem.h"
#include <assert.h>

void order_t :: read(BinFile &term)
{
   term >> id >> sum;
}

void order_t :: write(BinFile &term) const
{
   term << id << sum;
}

PQuelem * PQuelem::clone () const
{
   assert(0);
   return NULL;
}

PQuelem * PQuelem::read_clone (BinFile &, TSPSolver *) const
{
   assert(0);
   return NULL;
}

void PQuelem::print () const
{
   dump << "DUMB Quelem\n";
}

void PQuelem::write (BinFile &) const
{
   assert(0);
}

PQuelem::~PQuelem ()
{
}
