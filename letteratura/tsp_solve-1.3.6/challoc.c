/* CHalloc.c
 *
 * Inspired by Clint Staley's SmartAlloc
 *
 * orig ceh 5-10-93
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* USE_FAST_WHERES is only for use for non-reentrant allocations
 */
#define USE_FAST_WHERES
/*
#define USE_CHREALLOC
 */
#define QUICKY

#ifdef __cplusplus
extern "C" {
#endif

extern void *memset(void *, int, size_t);

/* debugging print #defines
#define PRER2
#define PRER
#define PRER_DEF
 */

#define EDGE_BYTES 8
/*
#define DEFAULT_FILL_LONG 0x0L
#define DEFAULT_FILL ((char)0x0)
 */
#define DEFAULT_FILL_LONG 0xa0a0a0aL
#define DEFAULT_FILL ((char)0xa)
#if (EDGE_BYTES < 4)
error
#endif

extern int fprintf(FILE*, const char *, ...);
extern int fclose(FILE*);
void _report_space(const int, const char *file, const int line);

/* when challoc() is called it must find the file and line number
 * in the wheres list.  If the file and line tuple is not found
 * a new one is created.
 */
typedef struct where_info_t {
   const char *file;
   int line;
   unsigned int space;
   unsigned long allocs;
   struct where_info_t *next, *prev;
} where_info_t;

static where_info_t *wheres = NULL;
static char *st_attempt =
   "R*%12.12s:%-4d Attempt to free non-malloced space %10lx\n";

/* locate_where() does not return a where_info_t as you would expect.
 * to let the caller know where the file and line tuple is, it
 * modifies the wheres list, by making the first where_info_t,
 * i.e. "wheres", the where_info_t that is associated with the
 * file and line passed.
 *
 * Always placing the where_info_t at the front of the list
 * is a way of speeding up mallocs when you have a bunch of
 * mallocs they are usually called on the same line in a loop.
 */
#ifdef USE_FAST_WHERES
#define _Where wheres
static void
#else
static where_info_t*
#endif
   locate_where(const char *file, const int line, const unsigned int space)
{
   where_info_t *tmp;

   for (tmp = wheres; tmp != NULL; tmp = tmp->next) {
      if (tmp->line == line && tmp->file == file && tmp->space == space) {
/*FIX-the filename comparison may have to be strcmp*/
         tmp->allocs++;
#ifdef PRER2
fprintf(stderr, 
"Whereing %s:%d %ld %u\n", tmp->file, tmp->line, tmp->allocs, tmp->space);
#endif
         if (tmp != wheres) {
            tmp->prev->next = tmp->next;
            if (tmp->next != NULL)
               tmp->next->prev = tmp->prev;
            tmp->next = wheres;
            wheres->prev = tmp;
            tmp->prev = NULL;
            wheres = tmp;
         }
#ifdef USE_FAST_WHERES
         return;
#else
         return wheres;
#endif
      }
   }
   tmp = (where_info_t*)malloc(sizeof(where_info_t));
   tmp->file = file;
   tmp->line = line;
   tmp->space = space;
   tmp->allocs = 1;
   if ((tmp->next = wheres) != NULL)
      wheres->prev = tmp;
   tmp->prev = NULL;
#ifdef PRER2
fprintf(stderr,
"New Wher %s:%d %ld %u\n", tmp->file, tmp->line, tmp->allocs, tmp->space);
#endif
#ifndef USE_FAST_WHERES
   return
#endif
   wheres = tmp;
}

static void free_where(where_info_t *to_free)
{
#ifdef PRER2
fprintf(stderr, "Free Whr %s:%d %ld %u\n",
to_free->file, to_free->line, to_free->allocs, to_free->space);
#endif
   if (--(to_free->allocs) == 0) {
      if (to_free->next != NULL) {
         to_free->next->prev = to_free->prev;
      }
      if (to_free->prev != NULL)
         to_free->prev->next = to_free->next;
      else {
         assert(to_free == wheres);
         if ((wheres = to_free->next) != NULL)
            wheres->prev = NULL;
      }
      free(to_free);
   }
}

/* the typedef for the structure that holds malloc allocations of
 * any size.  I call the allocs allocated in this structre
 * default_allocs because they are not blocked together by default.
 * If install_blocked_mallocs() is never called then all mallocs
 * use this structure by default.
 */
typedef struct allocation_t {
   char *data;
   where_info_t *where;
   unsigned int space;
   struct allocation_t *next;
} allocation_t;

/* allocation_block_t is the other way malloced allocations can be held.
 * This structure defines a list of allocations that were encompassed
 * by one internal malloc call.  The "data_block" represents a list of the
 * allocations used by the caller.  The block also includes in each element
 * of its list two addition fields that help map the allocations.  One
 * pointer is to the where_info_t to tell where the allocation was malloced
 * from in the callers code.  Another pointer is equal to the address of
 * the allocation_block that this allocation resides in.
 * This pointer can be used to find the allocation
 * block from only the memory address of the allocation.  chfree is one
 * procedure which is passed the address of the allocation and it needs
 * a way to get to the allocation block the allocation resides in.
 * chfree and similar procedures calculate the allocation block pointer
 * by subtracting 4 from the allocation pointer, (P), dereference it
 * and you get a 4 byte address, (A), telling where the allocation_block_t is.
 * You can find the index of the allocation in the allocation_block_t
 * with (A-*A)/(A->space+sizeof(allocation_block_t*)+sizeof(where_info_t*)):
 * the difference between the head of the block and the block element divided
 * by the size of the element in this case it is the space of the allocation
 * plus the size of two pointers.  The respective where_info_t can be
 * found a similar way except you need to subtract 8 (or the size of
 * two pointers) and then dereference.
 *
 * You can tell if the allocation is freed or allocated if the where_info_t
 * pointer is NULL or not NULL respectively inside the allocation.
 *
 * The prev pointer will point you to the previous allocation_block's
 * next pointer
 *
 * The free_ia pointer points to a free internal allocation block.
 * If the free_ia is NULL, then you have to manually search the
 * data block for a internal allocation with a NULL 'where'.
 */
typedef struct allocation_block_t {
   char *data_block;                    /* the data list of allocs        */
   struct allocation_block_t *next;     /* the next allocation_block      */
   struct allocation_block_t **prev;    /* addr of prev's next            */
   struct allocation_block_list_t *abl; /* the abl this block is in       */
   char *free_ia;                     /* the last free intr. allocation */
   unsigned short num_alloced;          /* number of unfreed allocs       */
} allocation_block_t;

/* an easier way to access the elements in the data_block list, rather
 * than casting all over the palce.
 */
#define INTERNAL_TEMP_DATA 4
typedef struct {
   where_info_t *where;
   allocation_block_t *ab;
   char data[INTERNAL_TEMP_DATA];
} internal_allocation_t;

/* the size of the internal allocation, not counting the data stored
 */
#define IA_HDR_SIZE (short)(sizeof(internal_allocation_t)-INTERNAL_TEMP_DATA)

/* the allocation_block list is to separate the installed blocked mallocs
 * from each other.
 *
 * the last_free pointer points to the last alloced or freed allocation
 * block so when trying to alloc new space and there are many full
 * allocation blocks it won't continuously search through them, it
 * could go right to the one it knows there is a free internal_alloc.
 * If this is NULL, then the last call to challoc filled an alloc_block_t
 * and the last_free block is unkown.  This means a search must be
 * performed to find an alloc_block_t that has a free element.
 *
 */
typedef struct allocation_block_list_t {
   allocation_block_t *blocks;        /* the blocks of this element       */
   unsigned short space;                /* size of each alloc in each block */
   unsigned short max_allocs;         /* max num internals per aloc_block */
   allocation_block_t *last_free;     /* if last free is NULL then search */
   struct allocation_block_list_t *next, *prev;
   const char *file; int line;        /* the location of installation     */
} allocation_block_list_t;

static allocation_t *default_allocs = NULL;
static allocation_block_list_t *list_of_blocked_allocs = NULL;

static void malloc_error(const char *file, const int line, const int space)
{
   fprintf(stderr, "%12.12s:%-4d Malloc Failure of %5d bytes\n",
      file, line, space);
   _report_space(1, file, line);
   exit(99);
}

void _install_blocked_challoc(unsigned short space, unsigned short max_allocs,
   const char *file, const int line)
{
   allocation_block_list_t *abl;

   if (max_allocs < 2 || space == 0)
      return;
   if (space%4)
      space += (unsigned short)(4-(space%4));
   for (abl = list_of_blocked_allocs; abl!=NULL; abl = abl->next)
      if (abl->space == space)
         return;
   if ((abl = (allocation_block_list_t*)
    malloc(sizeof(allocation_block_list_t)))==NULL)
      malloc_error("", 0, space);
#ifdef QUICKY
   if ((abl->next = list_of_blocked_allocs) != NULL)
      list_of_blocked_allocs->prev = abl;
   abl->prev = NULL;
#else
   abl->next = list_of_blocked_allocs;
#endif
   list_of_blocked_allocs = abl;
   abl->blocks = NULL;
   abl->space = space;
   abl->max_allocs = max_allocs;
   abl->file = file;
   abl->line = line;
   abl->last_free = NULL;
}

/*
static void print_bounds_error(const char *file, const int line)
{
   fprintf(stderr, "%12.12s:%-4d has data written past bounds!!!\n",
      file, line);
}
 */
#define print_bounds_error(_file,_line) \
   fprintf(stderr, "%12.12s:%-4d has data written past bounds!!!\n", \
      _file, _line);

/* this function is called to get an open internal allocation in
 * the alloc block.  We need to pass the space since the allocation_block_t
 * doesn't have the space we need to cycle through the internal allocation.
 * The where passed the the where_info_t where the source allocation occured.
 *
 * Point *1*: Since we assume there should be one free block here because we
 * assume (ab->num_alloced < ab->allocs), if all the wheres are non-NULL
 * the where's have been corrupted and we exit.
 */
static char *alloc_in_allocation_block(allocation_block_t *ab
#ifdef USE_FAST_WHERES
   )
#else
   , where_info_t *_Where)
#endif
{
   internal_allocation_t *ia;
   char *data, *end_data;
   unsigned short data_size, max_allocs=ab->abl->max_allocs;
#ifdef PRER
   unsigned short a_count;
#endif

   data_size = (unsigned short)(IA_HDR_SIZE+ab->abl->space);
   if ((data = ab->free_ia) != NULL) {
      if (data-(data_size*(max_allocs-1)) != ab->data_block
       && ((internal_allocation_t*)(data+data_size))->where == NULL)
         ab->free_ia = data+data_size;
      else
         ab->free_ia = NULL;
      ((internal_allocation_t*)(data))->where = _Where;
      if (++ab->num_alloced == max_allocs)
         ab->abl->last_free = NULL;
#ifdef PRER
fprintf(stderr, "%25.25s: %10lx %d\n", "Last Free Alloc in block",
(unsigned long)(data+IA_HDR_SIZE), (int)(data-ab->data_block)/data_size );
assert((data-ab->data_block)/data_size < max_allocs);
#endif
      return data+IA_HDR_SIZE;
   }
   end_data = (data = ab->data_block) + data_size*max_allocs;
#ifdef PRER
   for (a_count=0; data < end_data; data += data_size, a_count++) {
#else
   for (; data < end_data; data += data_size) {
#endif
      ia = (internal_allocation_t*)data;
      if (ia->where == NULL) {
         if (ia->ab != ab) {
            print_bounds_error(_Where->file, _Where->line);
            ia->ab = ab;
         }
         ia->where = _Where;
         if (++ab->num_alloced == max_allocs)
            ab->abl->last_free = NULL;
         if (data+data_size < end_data
          && ((internal_allocation_t*)(data+data_size))->where == NULL)
            ab->free_ia = data+data_size;
#ifdef PRER
fprintf(stderr, "%25.25s: %10lx %d\n", "Normal Alloc in block",
(unsigned long)(data+IA_HDR_SIZE), a_count);
#endif
         return data+IA_HDR_SIZE;
      }
   }
   fprintf(stderr, "Allocation Block is corrupted: Crashing\n");  /*1*/
   exit(1);
   return NULL;
}

/* makes a new allocation block and returns the first free data
 * allocation in the block.  returns the location that is returned to
 * the caller as the allocation needed.
 */
static char *make_allocation_block(allocation_block_list_t *abl
#ifdef USE_FAST_WHERES
   )
#else
   , where_info_t *_Where)
#endif
{
   allocation_block_t *ab;
   char *data,*end_data;
   internal_allocation_t *ia;
   int data_size;

   if ((ab=(allocation_block_t*)malloc(sizeof(allocation_block_t))) == NULL)
      malloc_error(_Where->file, _Where->line, abl->space);
#ifdef PRER
fprintf(stderr, "%25.25s: %10lx\n", "New Block", (unsigned long)(ab));
#endif
   ab->abl = abl;
   ab->num_alloced = 1;
   data_size = IA_HDR_SIZE+abl->space;
   if ((ab->data_block = (char*)malloc(data_size*abl->max_allocs)) == NULL)
      malloc_error(_Where->file, _Where->line, abl->space);
   memset(ab->data_block, DEFAULT_FILL, data_size*abl->max_allocs);
   data = ab->data_block;
   end_data = data + data_size*abl->max_allocs;
   ia = (internal_allocation_t*)data;
   ia->where = _Where;
   ia->ab = ab;
   for (data += data_size; data < end_data; data += data_size) {
      ia = (internal_allocation_t*)data;
      ia->where = NULL;
      ia->ab = ab;
   }
   ab->prev = &abl->blocks;
   if ((ab->next = abl->blocks)!=NULL)
      ab->next->prev = &ab->next;
   abl->blocks = ab;
#ifdef PRER
fprintf(stderr, "%25.25s: %10lx %d\n", "Normal Alloc in block",
(unsigned long)(ab->data_block+IA_HDR_SIZE), 0);
#endif
   abl->last_free = ab;
   ab->free_ia = ab->data_block+data_size;
   return ab->data_block+IA_HDR_SIZE;
}

/* search for an allocation.  new_allocation() searches for a free allocation.
 * It first looks though the blocked_allocs, If it does not find any in
 * the blocked mallocs (matching the size) then it looks at the separate
 * allocs, if not found it allocates a new allocation_t and returns it.
 * If new_allocation() returns NULL, then malloc() failed durring its call.
 */
void *challoc(const unsigned int space, const char *file,
   const int line, const char fill)
{
   allocation_block_list_t *abl;
   allocation_block_t *ab;
   allocation_t *a;
   char *data;
#ifndef USE_FAST_WHERES
   where_info_t *_Where =
#endif

   locate_where(file, line, space);
   for (abl = list_of_blocked_allocs; abl != NULL; abl = abl->next) {
      if (abl->space >= space && abl->space-4 < space) {
         if (abl->last_free != NULL) {
#ifdef QUICKY
            if (abl != list_of_blocked_allocs) {
               if (abl->next != NULL)
                  abl->next->prev = abl->prev;
               assert(abl->prev != NULL);
               abl->prev->next = abl->next;
               abl->prev = NULL;
               abl->next = list_of_blocked_allocs;
               list_of_blocked_allocs = list_of_blocked_allocs->prev = abl;
            }
#endif
#ifdef USE_FAST_WHERES
            return alloc_in_allocation_block(abl->last_free);
#else
            return alloc_in_allocation_block(abl->last_free, _Where);
#endif
         }
         for (ab = abl->blocks; ab != NULL; ab = ab->next) {
            if (ab->num_alloced < abl->max_allocs) {
#ifdef USE_FAST_WHERES
               return alloc_in_allocation_block(ab);
#else
               return alloc_in_allocation_block(ab, _Where);
#endif
            }
         }
#ifdef USE_FAST_WHERES
         return make_allocation_block(abl);
#else
         return make_allocation_block(abl, _Where);
#endif
      }
   }
   if ((data = (char*)malloc(space+2*EDGE_BYTES)) == NULL)
      malloc_error(file, line, space);
   if (fill == DEFAULT_FILL)
      memset(data, DEFAULT_FILL, space+2*EDGE_BYTES);
   else {
      memset(data+EDGE_BYTES, fill, space);
      memset(data, DEFAULT_FILL, EDGE_BYTES);
      memset(data+space+EDGE_BYTES, DEFAULT_FILL, EDGE_BYTES);
   }
   if ((a = (allocation_t *)malloc(sizeof(allocation_t)) )==NULL)
      malloc_error(file, line, space);
   a->next = default_allocs;
   default_allocs = a;
   a->data = data;
   a->space = space;
   a->where = _Where;
#ifdef PRER_DEF
fprintf(stderr, "%25.25s: %10lx\n", "Default Allocation",
(unsigned long)(data+EDGE_BYTES));
#endif
   return data+EDGE_BYTES;
}

/* this is the function which frees an blocked allocation, this can easily
 * segfault if the user program modifies its internal data (outside the
 * space given to the user by challoc.
 */
static void free_internal_blocked_allocation(char *data, const char *file,
   const int line)
{
   internal_allocation_t *ia;
   allocation_block_t *ab;

   ia = (internal_allocation_t *)(data - IA_HDR_SIZE);
   ab = ia->ab;
#ifdef PRER
fprintf(stderr, "%25.25s: %10lx %d\n", "Free From Block",
(unsigned long)(data),
(int)(data-IA_HDR_SIZE-ab->data_block)/(IA_HDR_SIZE+ab->abl->space));
assert((data-IA_HDR_SIZE-ab->data_block)/(IA_HDR_SIZE+ab->abl->space)
< ab->abl->max_allocs);
#endif
   if (ab->num_alloced == 0 || ia->where == NULL) {
      fprintf(stderr, st_attempt+2, file, line, (long)data);
      return;
   }
   memset(ia->data, DEFAULT_FILL, ia->where->space);
   free_where(ia->where);
   ia->where = NULL;
   if (--ab->num_alloced == 0) {
#ifdef PRER
fprintf(stderr, "%25.25s: %10lx\n", "Delete Block", (unsigned long)(ab));
#endif
      if (ab->next != NULL)
         ab->next->prev = ab->prev;
      *ab->prev = ab->next;
      if (ab->abl->last_free == ab)
         ab->abl->last_free = NULL;
      free(ab->data_block);
      free(ab);
   }
   else
      ab->abl->last_free = ab;
}

/* frees the challoc
 *
 * Point *1*: assumes if the 4 bytes before the address are default
 * filled, then this alloc is a blocked malloc and will try to
 * dereference the pointers before the challoc internal_allocation_t.
 * This could will segfault if the data inside it has been modified
 * unknowingly by the user program.
 */
void chfree(void *dat, const char *file, const int line)
{
   allocation_t **to_free_ptr, *to_free;
   char *data_address;
   int data_size;
   char *end, *data;

   data_address = (char*)dat - EDGE_BYTES;
   if (*(long *)data_address != DEFAULT_FILL_LONG) { /*1*/
      free_internal_blocked_allocation((char*)dat, file, line);
      return;
   }
   to_free_ptr = &default_allocs;
   while (*to_free_ptr != NULL && (*to_free_ptr)->data != data_address)
      to_free_ptr = &(*to_free_ptr)->next;
   if ((to_free = *to_free_ptr) == NULL) {
      fprintf(stderr, st_attempt+1, file, line, (long)dat);
      return;
   }
   data_size = to_free->space + EDGE_BYTES;
   for (data=data_address, end = data+EDGE_BYTES; data < end; data++) {
      if (*(data + data_size) != DEFAULT_FILL || *data != DEFAULT_FILL) {
         fprintf(stderr, "%12.12s:%-4d trying to free data_size %19lx\n",
            file, line, (long)dat);
         print_bounds_error(to_free->where->file, to_free->where->line);
         break;
      }
   }
   *to_free_ptr = to_free->next;
   memset(dat, DEFAULT_FILL, data_size-EDGE_BYTES);
#ifdef PRER_DEF
fprintf(stderr, "%25.25s: %10lx\n", "Default Free", (unsigned long)(dat));
#endif
   free(data_address);
   free_where(to_free->where);
   free(to_free);
}

#ifdef USE_CHREALLOC
void *chrealloc(void *dat, unsigned int space, const char *file,
   const int line, const int fill)
{
   allocation_t **to_free_ptr, *to_free;
   char *data_address;
   unsigned int old_space;
   char *data;
   internal_allocation_t *ia;

   data_address = (char*)dat - EDGE_BYTES;
   if (*(long *)data_address != DEFAULT_FILL_LONG) {    /*1*/
      ia = (internal_allocation_t *)((char*)dat - IA_HDR_SIZE);
      old_space = (ia->where == NULL ? 0 : ia->where->space);
   }
   else {
      to_free_ptr = &default_allocs;
      while (*to_free_ptr != NULL && (*to_free_ptr)->data != data_address)
         to_free_ptr = &(*to_free_ptr)->next;
      if ((to_free = *to_free_ptr) == NULL)
         old_space = 0;
      else {
         if (default_allocs != to_free) {
            *to_free_ptr = to_free->next;
            to_free->next = default_allocs;
            default_allocs = to_free;
         }
         old_space = to_free->space;
      }
   }
   data = challoc(space, file, line, fill);
   if (old_space == 0)
      fprintf(stderr, st_attempt, file, line, (long)dat);
   else {
      memcpy(data, dat, ((space < old_space) ? space : old_space));
      chfree(dat, file, line);
   }
   return data;
}
#endif

void _report_space(const int parm, const char *file, const int line)
{
   allocation_t *an_alloc;
   allocation_block_list_t *abl;
   allocation_block_t *ab;
   internal_allocation_t *ia;
   unsigned int data_size;
   unsigned short a_count;
   char *data, *end_data;
   unsigned long tmp_space, tmp_pieces, space=0, pieces=0, num_block;
   where_info_t *tmp;
   FILE *bunk;
   
   if (parm < 3 || (bunk=fopen("chalstat", "w"))==NULL)
      bunk = stderr;

   for (abl = list_of_blocked_allocs; abl != NULL; abl = abl->next) {
      tmp_space = abl->space;
      tmp_pieces = num_block = 0;
      for (ab = abl->blocks; ab != NULL; ab = ab->next, num_block++) {
         data = ab->data_block;
         data_size = IA_HDR_SIZE+(unsigned int)tmp_space;
         end_data = data + data_size*abl->max_allocs;
         for (a_count = 0; data < end_data; data += data_size) {
            ia = (internal_allocation_t*)data;
            if (ia->where != NULL) {
               a_count++;
               tmp_pieces++;
               if (parm >= 2) {
                  fprintf(bunk, "%12.12s:%-4d has unfreed space of",
                     ia->where->file, ia->where->line);
                  fprintf(bunk, " %5d bytes%13lx==*%lx\n", (int)tmp_space,
                     (long)&ia->data+EDGE_BYTES, (long)&ia->data);
               }
            }
            if (ia->ab != ab) {
               if (ia->where != NULL)
                  fprintf(bunk, "%12.12s:%-4d",
                     ia->where->file, ia->where->line);
               fprintf(bunk, "Alloc Corrupted, Tried to Fix\n");
               ia->ab = ab;
            }
         }
         if (a_count != ab->num_alloced) {
            fprintf(bunk, "Num Allocs do not compare in block\n");
            ab->num_alloced = a_count;
         }
      }
      if (parm > 0 && tmp_pieces != 0) {
         fprintf(bunk, "%5lu size-[%3lu, %3lu] Blocks contain %-5lu unfreed pieces. %s:%d\n", num_block, tmp_space, (unsigned long)abl->max_allocs, tmp_pieces, abl->file, abl->line);
      }
      space += tmp_pieces*tmp_space;
      pieces += tmp_pieces;
   }
   tmp_pieces = tmp_space = 0;
   for (an_alloc=default_allocs; an_alloc!=NULL; an_alloc=an_alloc->next) {
      if (parm < 0) {
         data = an_alloc->data;
         data_size = an_alloc->space + EDGE_BYTES;
         for (end_data = data+EDGE_BYTES; data < end_data; data++) {
            if (*(data + data_size) != DEFAULT_FILL || *data != DEFAULT_FILL) {
               fprintf(bunk, "%12.12s:%-4d Bounds Check Failed %9lx\n",
                  file, line, (long)data);
               print_bounds_error(an_alloc->where->file, an_alloc->where->line);
               break;
            }
         }
      }
      if (parm >= 2) {
         fprintf(bunk, "%12.12s:%-4d has unfreed space of",
            an_alloc->where->file, an_alloc->where->line);
         fprintf(bunk, " %5d bytes%13lx==*%lx\n", an_alloc->space,
            (long)an_alloc->data+EDGE_BYTES, (long)&an_alloc->data);
      }
      tmp_pieces++;
      tmp_space += an_alloc->space;
      space += an_alloc->space;
   }
   pieces += tmp_pieces;
   if (parm > 0 && tmp_pieces != 0)  {
      fprintf(bunk, "Default Unfreed Space is %ld in %ld pieces\n",
         tmp_space, tmp_pieces);
   }
   if (pieces != 0) {
      fprintf(bunk, "Total Unfreed Space is %ld in %ld pieces\n",
         space, pieces);
   }
   else {
      if (wheres != NULL)
         fprintf(bunk, "\n****WARNING*****CHALLOC*ERROR****\n");
   }
   if (parm > 0) {
      for (tmp = wheres; tmp != NULL; tmp = tmp->next) {
         allocation_block_list_t *abl = list_of_blocked_allocs;
         int d = 0;
         for (; abl != NULL; abl = abl->next)
            if (abl->space == tmp->space + (4-(tmp->space%4))%4)
               d = 1;
         fprintf(bunk, "%12.12s:%-4d has %6ld%c allocs of size %u\n",
            tmp->file, tmp->line, tmp->allocs, d?' ':'*', tmp->space);
      }
   }
   if (bunk != stderr)
      fclose(bunk);
}

#ifdef __cplusplus
};
#endif
