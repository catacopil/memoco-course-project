/* CHnew.cc
 *
 * No don't say godblessyou.  This a C++ garbage collection utility.
 *
 * orig ceh Aug 93
 */

/* USE_CHALLOC will enable challocs from being called
 * USE MALLOC will call malloc or a challoc macro ifdef USE_CHALLOC
#define USE_MALLOC
 */
#define USE_CHALLOC

#ifdef MDEBUG // to stop Overloading new when not using MDEBUG

#include <stdlib.h>
#include <stdio.h>
#ifdef USE_CHALLOC
# include "challoc.h"
# undef malloc
# undef free
#endif

// a list of all the new spaces that were not called in regular .cc files
typedef struct internal_new_t {
   void *data;
   struct internal_new_t *next, **prev;
} internal_new_t;

// The desctructor call stack is so that inherited destructors call will
// preserve filaname and line number whilest recursively calling delete
typedef struct dcall_stack_t {
   const char *filename;
   int line;
   struct dcall_stack_t *next;
} dcall_stack_t;

static dcall_stack_t *dcall_stack=NULL;
static internal_new_t *inews=NULL;
char *__fileNAME;
int __lineNUM;

void _push_dcall_stack(const char *filename, const int line)
{
   dcall_stack_t *push = (dcall_stack_t*)malloc(sizeof(dcall_stack_t));

   if (push == NULL) {
      fprintf(stderr, "Malloc Failure in name stack\n");
      exit(1);
      return;
   }
   push->next = dcall_stack;
   push->filename = filename;
   push->line = line;
   dcall_stack = push;
}

static void pop_dcall_stack(char * &filename, int &line)
{
   dcall_stack_t *poped = dcall_stack;

   filename = (char *)poped->filename;
   line = poped->line;
   dcall_stack = poped->next;
   free(poped);
}

void *operator new(size_t size)
{
   void *ret;
   internal_new_t *inew;

   if (__fileNAME==NULL) {
      int after_ret = ((size%4)?(4-size%4):0);
      ret = (void*)malloc(size + after_ret + sizeof(internal_new_t));
      inew = (internal_new_t*)((char*)ret + size + after_ret);
      inew->data = (void*)ret;
      inew->next = inews;
      if (inews != NULL) {
         inews->prev = &(inew->next);
      }
      inew->prev = &inews;
      inews = inew;
   }
   else {
#ifdef USE_MALLOC
      ret = (void*)malloc(size);
#else
      ret = challoc(size, __fileNAME, __lineNUM, DEFAULT_FILL);
#endif
      __fileNAME = NULL;
   }
   return ret;
}

// Point *1*: Ansi C++ says ok to delete NULL
void operator delete(void *object)
{
   char *filename;
   int line;
   internal_new_t *inew;

   if (object != NULL) { /*1*/
      for (inew = inews; inew != NULL && inew->data != object; inew = inew->next)
         ;
/*
         if (inew->next == (internal_new_t*)DEFAULT_FILL_LONG)
            inew = NULL;
 */
      if (inew != NULL) {
         *(inew->prev) = inew->next;
         free(inew->data);
      }
      else {
         pop_dcall_stack(filename, line);
#ifdef USE_MALLOC
         free(object);
#else
         chfree(object, filename, line);
#endif
      }
   }
}

#endif
