#include "matrix.h"
#include "params.h"
#include "chnew.h"
#include "findtour.h"

class CMatrix : public Matrix
{
   CMatrix(char *);
   CMatrix();
}

int main(int argc, char *argv[])
{
   Timer *tm = new Timer;
   char nm[20];

   param.init(argc, argv);
   if (param.degree < 7)
      param.degree = 7;

   Matrix *matrix;
   if (param.filename != NULL) {
      matrix = new CMatrix(param.filename);
   }
   else {
      matrix = new CMatrix();
   }
   FindTour *h;
   h = new FindTour("ant", matrix, tm, 0, NULL);
   h->tour->print_oneway(dump);
   dump << "the cost is:\n" << h->length << "\n";
   dump << "the clock time is:\n";
   tm->print();
   dump << "\n";

   delete h;
   delete matrix;
   delete tm;
   dump.Term::~Term();
   report_space(1);
   return 0; 
}
