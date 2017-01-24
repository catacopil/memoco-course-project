/* io.cc
 *
 * a better way than including <iostream.h> in all the files
 *
 * orig ceh
 */

#include "io.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <errno.h>
#include "chlib.h"
#include "chnew.h"
#define fequiv(_a,_b,_c) (fabs((double)(_a)-(_b))<(double)(_c))

#define STATUS_EOF 0x1

#ifndef HAS_TIME_DECL
extern "C" {
   extern time_t time(time_t*);
}
#endif

// return if two character pointers (reguardless of sign) are equal
#define CEQ(_c1,_c2) cequal((const char)(_c1), (const char)(_c2))
static int cequal(const char c1, const char c2)
{
   return c1==c2;
}

static int is_word_char(const char *_c)
{
   return !isspace(*_c);
}
 
// return if the digit is or is not a number
static int is_number_char(const char *_c)
{
   int ret;
   if (isdigit(*_c))
      ret = 1;
   else {
      switch (*_c) {
      case 'e': case 'E':
         ret = (is_number_char(_c-1) && is_number_char(_c+1));
         break;
      case '+': case '-': case '.':
         ret = (_c[-1] == 'e' || _c[-1] == 'E' || is_number_char(_c-1)
          || _c[1] == 'e' || _c[1] == 'E' || is_number_char(_c+1));
         break;
      default:
         ret = 0;
         break;
      }
   }
   return ret;
}

Term dump(0), dumpout(1);

#define NEW_BUFFER(_x, _i) _x = new char[(_i)+1], *_x++ = 0
#define DELETE_BUFFER(_x) --(_x), delete (_x)
#define INITMAXTERMBUF 80
#define TERMBUFINC     20

int Term :: eof()
{
   return status & STATUS_EOF;
}

Term :: Term(int i)
{
   fp = i ? stdout : stderr;
   status = 0;
   lockname = NULL;
   name = "-";
   NEW_BUFFER(buf, INITMAXTERMBUF);
   maxtermbuf = INITMAXTERMBUF;
   bufptr = NULL;
}

Term :: ~Term()
{
   close();
   if (buf != NULL) {
      DELETE_BUFFER(buf);
      buf = NULL;
   }
}

Term& Term :: form(const char *fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   vfprintf((FILE*)fp, fmt, ap);
   va_end(ap);
   return *this;
}

int Term :: open(const char *filename, const char *mode, int withlock)
{
   int ret = 0;

   if (fp != stdout && fp != stderr)
      return 1;
   status = 0;
   name = filename;
   if (withlock) {
      lockname = lockfile(filename, 0);
      if (lockname == NULL) {
         name = NULL;
         fp = (void*)fopen("/dev/null", mode);
         if (fp == NULL)
            fp = stderr;
         dump << filename << ": Open Error 2\n";
         return 2;
      }
   }
   fp = (void*)fopen((name==NULL) ? "/dev/null" : filename, mode);
   if (fp == NULL || name==NULL) {
      unlockfile(lockname);
      name = lockname = NULL;
      if (fp == NULL)
         fp = (void*)fopen("/dev/null", mode);
      if (fp == NULL)
         fp = stderr;
      ret = (errno == ENOENT) ? 3 : 1;
      dump << filename << ": Open Error " << ret << "\n";
   }
   return ret;
}

void Term :: close()
{
   if (fp != stdout && fp != stderr) {
      unlockfile(lockname);
      lockname = NULL;
      fclose((FILE*)fp);
      fp = stderr;
      name = NULL;
   }
   status = 0;
}

void Term :: close(const char *filename)
{
   if (fp != stdout && fp != stderr && strncmp(filename, name, 16)==0) {
      fclose((FILE*)fp);
      fp = stderr;
      name = NULL;
   }
}

void Term :: flush()
{
   fflush((FILE*)fp);
}

Term& Term :: operator << (const int d)
{
   fprintf((FILE*)fp, "%d", d);
   return *this;
}

Term& Term :: operator << (const unsigned int d)
{
   fprintf((FILE*)fp, "%u", d);
   return *this;
}

Term& Term :: operator << (const pos_t &d)
{
   /*
   form("%.20lf %.20lf\n", d.x, d.y);
   return *this;
    */
   return *this << d.x << " " << d.y << "\n";
}

Term& Term :: operator << (const double d)
{
   if (fequiv(d, (long)d, .0000001))
      fprintf((FILE*)fp, "%.0f", d);
   else
      fprintf((FILE*)fp, "%f", d);
   return *this;
}

Term& Term :: operator << (const signed short s)
{
   fprintf((FILE*)fp, "%hd", s);
   return *this;
}

Term& Term :: operator << (const signed long v)
{
   fprintf((FILE*)fp, "%ld", v);
   return *this;
}

Term& Term :: operator << (const unsigned short s)
{
   fprintf((FILE*)fp, "%hu", s);
   return *this;
}

Term& Term :: operator << (const unsigned long v)
{
   fprintf((FILE*)fp, "%lu", v);
   return *this;
}

Term& Term :: operator << (const unsigned char v)
{
   return *this << (const unsigned int)v;
}

Term& Term :: operator << (const char v)
{
/*
   if (v < 20)
      return *this << (const unsigned int)v;
 */
   fprintf((FILE*)fp, "%c", v);
   return *this;
}

Term& Term :: operator << (const signed char v)
{
   return *this << (const unsigned int)v;
}

char* Term :: tfgets(char *gbuf, int N)
{
   int y=0, x=0;
   if (!(bufptr == NULL || !is_number_char(bufptr) || CEQ(*bufptr, 0))) {
      while (!CEQ(bufptr[y], 0) && x != N-1) {
         gbuf[x] = bufptr[y];
         x++;
         y++;
      }
      gbuf[x] = 0;
      bufptr = bufptr+y;
      return gbuf;
   }
   bufptr = NULL;
   return fgets(gbuf, N, (fp != stdout && fp != stderr) ? (FILE*)fp : stdin);
}

#define TIMES_TIMEOUT 2

// Point *1*: zero out the first char just incase
// fgets forgets to upon NULL return.
//
char *Term :: get_string(int (*is_char)(const char *))
{
   char *ptr;
   int times=0;

   if (bufptr != NULL)
      while (!CEQ(*bufptr, 0) && !is_char(bufptr))
         bufptr++;
   while (bufptr == NULL || !is_char(bufptr) || CEQ(*bufptr, 0)) {
      buf[maxtermbuf-2] = 0;
      if (fgets(buf, maxtermbuf,
       (fp != stdout && fp != stderr) ? (FILE*)fp : stdin) == NULL) {
         buf[0] = 0; /*1*/
         if (times++ > TIMES_TIMEOUT) {
            dump << "End Of File\n";
            status |= STATUS_EOF;
            break;
         }
      }
      else {
         char *newbuf;
         while (!CEQ(buf[maxtermbuf-2], 0) && !CEQ(buf[maxtermbuf-2], '\n')) {
            NEW_BUFFER(newbuf, maxtermbuf+TERMBUFINC);
            memcpy(newbuf, buf, maxtermbuf);
            DELETE_BUFFER(buf);
            buf = newbuf;
            maxtermbuf += TERMBUFINC;
            buf[maxtermbuf-2] = 0;
            if (fgets(buf+maxtermbuf-TERMBUFINC-1, TERMBUFINC+1,
             (fp != stdout && fp != stderr) ? (FILE*)fp : stdin)==NULL) {
               buf[maxtermbuf-TERMBUFINC-1] = 0; /*1*/
               break;
            }
         }
      }
      for (bufptr = buf; !CEQ(*bufptr,0) && !is_char(bufptr); bufptr++)
         ;
   }
   /* assume bufptr points to a string that will end up with a digit */
   ptr = bufptr;
   while (is_char(bufptr))
      bufptr++;
   while (!CEQ(*bufptr, 0) && isspace(*bufptr))
      bufptr++;
   return ptr;
}

Term& Term :: operator >> (char *(&word))
{
   char *ptr, *save;
   if (word != NULL)
      delete word;
   save = get_string(is_word_char);
   for (ptr = save; !isspace(*ptr); ptr++)
      ;
   if (ptr-save > 0 && *save==NULLCHAR)
      word = NULL;
   else {
      word = new char[(int)(ptr-save)+1];
      for (ptr = save; !isspace(*ptr); ptr++)
         word[(int)(ptr-save)] = *ptr;
      word[(int)(ptr-save)] = 0;
   }
   return *this;
}

Term& Term :: operator >> (signed long &s)
{
   s = atoi(get_string(is_number_char));
   return *this;
}

Term& Term :: operator >> (double &s)
{
   s = atof(get_string(is_number_char));
   return *this;
}

Term& Term :: operator << (const float f)
{
   fprintf((FILE*)fp, "%f", f);
   return *this;
}
Term& Term :: operator << (const unsigned char* word)
{
   return *this << (const char*)word;
}
Term& Term :: operator << (const char* word)
{
   char null[2]={NULLCHAR,0};
   fprintf((FILE*)fp, "%s", (word == NULL) ? null : word);
   return *this;
}

Term& Term :: operator << (const void *v)
{
   fprintf((FILE*)fp, "%lx", (const long)v);
   return *this;
}

void Term :: Time ()
{
   time_t Tm=time(0);

   fprintf((FILE*)fp, "%s", ctime(&Tm));
   return;
}
