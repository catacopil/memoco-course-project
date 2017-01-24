/* tour.cc
 *
 * The tsp tour class
 *
 * orig ceh
 */

#include "tour.h"
#include "circlist.h"
#include <assert.h>
#include "stdmacro.h"
#include "smatrix.h"

void Tour :: write(Term &t) const
{
   Path *p;
   int modu = 0;
   TourIter ti(*this);

   t << " " << degree << " " << size() << "\n";
   while ((p = ti.next()) != NULL) {
      t << " " << p->id;
      if (!(++modu%10)) {
         modu = 0;
         t << "\n";
      }
   }
   if ((++modu%10) != 1)
      t << "\n";
}

void Tour :: read(Term &t)
{
   long d1, new_listsize;
   city_id_t i, j;

   this->Tour::~Tour();
   t >> d1 >> new_listsize;
   degree = (city_id_t)d1;
   if (degree == 0)
      static_paths = NULL;
   else
      static_paths = new Path [degree];
   for (i = 0; i < new_listsize; i++) {
      t >> d1;
      j = (city_id_t)d1;
      circ.insert(get_new_path(j));
   }
}

void Tour :: write(BinFile &t) const
{
   Path *p;
   long new_listsize = size();
   TourIter ti(*this);

   t << degree << new_listsize;
   while ((p = ti.next()) != NULL)
      t << p->id;
}

void Tour :: append(Tour *t)
{
   Path *p;
   TourIter ti(*t);

   if (this == t)
      return;
   while ((p = ti.next()) != NULL)
      circ.insert(t->circ.del(p));
}

void Tour :: read(BinFile &t)
{
   city_id_t i, j;
   long new_listsize;

   this->Tour::~Tour();
   t >> degree >> new_listsize;
   if (degree == 0)
      static_paths = NULL;
   else
      static_paths = new Path [degree];
   for (i = 0; i < new_listsize; i++) {
      t >> j;
      circ.insert(get_new_path(j));
   }
}

void Tour :: copy(const Tour &t)
{
   Path *p;

   this->Tour::~Tour();
   construct(t.size());
   TourIter ti(t);
   while ((p = ti.next()) != NULL) {
      circ.insert(get_new_path(p->id));
   }
   length_cost = t.length_cost;
}

Tour :: Tour()
{
   static_paths = NULL;
   degree = 0;
   length_cost = MAX_SUM;
}

// Borland 3.1 doesn't like to allocate arrays of classes
// automatically with a single 'new'
void Tour :: construct(const city_id_t deg)
{
#ifdef NO_ARRAY_CONSTRUCTING
   city_id_t i;
   Path p;
   static_paths = (Path*)(new char[deg * sizeof(Path)]);
   for (i = 0; i < deg; i++)
      memcpy(static_paths+i, &p, sizeof(Path));
#else
   static_paths = new Path[deg];
#endif
   degree = deg;
   length_cost = MAX_SUM;
}

Tour :: Tour(const city_id_t deg)
{
   construct(deg);
}

Tour :: ~Tour()
{
   if (static_paths != NULL) {
      delete static_paths;
      static_paths = NULL;
      degree = 0;
   }
   else {
      Path *p;
      TourIter ti(*this);
      while ((p = ti.next()) != NULL)
         delete p;
   }
}

void Tour :: reverse()
{
   circ.reverse();
}

Path* Tour :: get_new_path(const city_id_t j)
{
   Path *p;
   if (static_paths != NULL)
      p = static_paths+j;
   else
      p = new Path;
   p->id = j;
   return p;
}

void Tour :: travel(city_id_t id)
{
   Path *p = get_new_path(id);
   circ.insert(p);
}

Path * Tour :: insert_after(Path *here, city_id_t id)
{
   Path *p = get_new_path(id);
   circ.insert_after(here, p);
   return p;
}

void Tour :: insert_before(city_id_t before_here, city_id_t here)
{
   Path *p;
   TourIter ti(*this);

   while ((p = ti.next()) != NULL) {
      if (p->get_next()->id == before_here)
         break;
   }
   insert_after(p, here);
}

int Tour :: is_complete() const
{
   return is_complete(degree);
}

int Tour :: is_complete(const city_id_t deg) const
{
   if (degree != deg || static_paths == NULL || size() != deg)
      return 0;
   city_id_t id, traveled=0;
   _uchar *checks = new _uchar[degree];

   for (id = 0; id < degree; id++)
      checks[id] = 0;
   for (id = 0; id < degree; id++) {
      if (checks[static_paths[id].id] != (char)0) {
         delete checks;
         return 0;
      }
      else {
         traveled++;
         checks[static_paths[id].id] = 1;
      }
   }
   delete checks;
   return traveled == degree;
}

static short st_mu[]={
   13462, 11372, 26383, 9945, 25500, 22756, 18622, 4600, 20743, 12641, 28815,
   5789, 18656, 16247, 26942, 28466, 2424, 29700, 23433, 12670, 15370, 25828,
   12962, 6083, 6383, 8189, 7216, 6378, 6021, 11095, 3338, 22183, 15007, 13208,
   15580, 21299, 18652, 14752, 30898, 15281, 1877, 24928, 20434, 18056, 14471,
   11160, 3403, 4586, 20364, 14123, 16257, 18954, 24030, 14751, 6974, 26129,
   30902, 17154, 11173, 18582, 16477, 28179, 17452, 18338, 2737, 25425, 20641,
   0
};

void Tour :: print_oneway(Term &term) const
{
   Path *n=NULL;
   short found=0, count=0;
   char c;

   while (st_mu[count] != found) {
      c = (char)(st_mu[count++] % 299);
      n = get_head();
      dump << c;
   }
   for (; n != NULL; n = n->get_next()) {
      if (found && count != 0) {
         term << n->get_prev()->id << " ";
         if (n->get_next()->id == degree-1) {
            assert(n->get_next()->get_next()->id == degree-2);
            break;
         }
      }
      else if (n->get_prev()->id == degree-2)
         found = 1;
   }
   if (n != NULL && count != 0)
      term << n->id;
   term << "\n";
}

/*
#define PRINTZEROCITY ((city_id_t)1)
 */
#define PRINTZEROCITY ((city_id_t)0)

void Tour :: print(Term &term) const
{
   Path *n, *p;
   TourIter ti(*this);
   int i;

   for (i = 0, p = ti.next(); (n = ti.next()) != NULL; p = n) {
      if (!((++i)%15))
         term << "\n";
      term << (p->id+PRINTZEROCITY) << ", ";
   }
   if (p != NULL)
      term << (p->id+PRINTZEROCITY);
   term << "\n";
}

void Tour :: print() const
{
   print(dump);
}

sum_t Tour :: cost(const Matrix *m)
{
   Path *p, *n;
   sum_t sum;

   if (length_cost != MAX_SUM)
      return length_cost;
   TourIter ti(*this);
   for (sum = 0, p = ti.next(); (n = ti.next()) != NULL; p = n) {
      sum += m->val(p->id, p->get_next()->id);
   }
   if (p != NULL) {
      sum += m->val(p->id, p->get_next()->id);
   }
   return length_cost = sum;
}

/*
 */
/*
#define SHOW_N
 */

void Tour :: show(const Matrix *m) const
{
   Path *p, *n;

#ifdef SHOW_N
SortedMatrix sm(degree);
SortedCost *sc;
city_id_t i;
sm.sort(m);
#endif

   if (!m->is_geometric_2d()) {
      print(dumpout);
      return;
   }
   TourIter ti(*this);
   for (p = ti.next(); (n = ti.next()) != NULL; p = n) {

#ifdef SHOW_N
sc = sm.cost[p->id];
for (i=0; i<degree; i++) {
 if (sc[i].id == p->get_next()->id) {
  dumpout << (const char)('A'+i) << " ";
  break;
 }
}
for (i=0; i<degree; i++) {
 if (sc[i].id == p->get_prev()->id) {
  dumpout << (const char)('A'+i) << " ";
  break;
 }
}
#endif

      dumpout << m->pos[p->id];
   }
   if (p != NULL) {
      dumpout << m->pos[p->id];
      dumpout << m->pos[p->get_next()->id];
   }
}

void Tour :: change_head_to(const city_id_t h)
{
   Path *p;
   TourIter ti(*this);
   while ((p = ti.next()) != NULL) {
      if (p->id == h)
         break;
   }
   assert(p!=NULL);
   circ.change_head_to(p);
}

void Tour :: change_head_to(const Path *p)
{
   circ.change_head_to(p);
}

Path *Tour :: get_head() const
{
   return (Path*)circ.get_head();
}

int Tour :: compare(Tour &T)
{
   Path *Tp, *p;
   TourIter ti(T), tis(*this);
   city_id_t i;
   int ret;

   p = tis.next();
   if (p != NULL) {
      for (i = p->id; (Tp = ti.next()) != NULL; ) {
         if (Tp->id == i)
            break;
      }
   }
   else
      Tp = NULL;
   if (Tp != NULL && (p = tis.next()) != NULL) {
      i = p->id;
      if (Tp->get_next()->id == i) {
         Tp = Tp->get_next();
         do {
            Tp = Tp->get_next();
         } while ((p = tis.next()) != NULL && p->id == Tp->id);
      }
      else if (Tp->get_prev()->id == i) {
         Tp = Tp->get_prev();
         do {
            Tp = Tp->get_prev();
         } while ((p = tis.next()) != NULL && p->id == Tp->id);
      }
      if (p != NULL)
         ret = 1;
      else
         ret = 0;
   }
   else
      ret = 1;
   return ret;
}
