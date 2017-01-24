/* io.h
 * replacement for "cout" because <iostream.h> is too much to put in every file
 * orig ceh
 */
#ifndef _IO_H
#define _IO_H

#include "config.h"
#include "chlib.h" // only for pos_t

// if character is read in as a string, then Term >> string; assigns string NULL
#define NULLCHAR '^'

class Term
{
   char *buf, *bufptr, *lockname;
   int maxtermbuf, status;
   char *get_string(int (*is_char)(const char *));
   void *fp;
   const char *name;
public:
   Term(int);
   ~Term();
   // open returns 0 if successful, 1 if lock was not successful,
   // 2 if file could not be physically opened
   // 3 if the file was opened for read but did not exist
   int open(const char* filename, const char *mode, int withlock);
   void close();
   void close(const char *filename);
   void Time();
   Term& operator << (const int);
   Term& operator << (const unsigned int);
   Term& operator << (const double);
   Term& operator << (const signed short);
   Term& operator << (const signed long);
   Term& operator << (const unsigned short);
   Term& operator << (const unsigned long);
   Term& operator << (const unsigned char);
   Term& operator << (const signed char);
   Term& operator << (const char);
   Term& operator << (const void *);
   Term& operator << (const float);
   Term& operator << (const char*);
   Term& operator << (const unsigned char*);
   Term& operator << (const pos_t &);
   void flush();
   Term& form (const char *fmt, ...);
   int eof();

   // input functions only grab numbers that will be tokenized from the
   // following regular expression: (([\.01-9+e\-]+)|([^\.01-9+e\-]+))*
   // then then leaves the stream cursor on the character after the last
   // whitespace character beyond the number's string.
   Term& operator >> (double &);
   Term& operator >> (signed long &);
   // deletes the word if it is non-null and news the string separated
   // by whitespace which is after the whitespace beyond the stream cursor
   Term& operator >> (char *(&word));

   // behaves identical to fgets and works "with" the input functions above
   char* tfgets(char*, int);
};

extern Term dump, dumpout;

#endif
