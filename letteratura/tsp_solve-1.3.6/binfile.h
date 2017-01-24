/* binfile.h
 *
 * orig ceh
 */
#ifndef _BINFILE_H
#define _BINFILE_H

#include "config.h"

// Bin written and read in Strings have a maximum size of MAXBINSTRING
// if the string saved is "^" then it is read in as NULL
#define MAXBINSTRING 100

class BinFile
{
   void *fp;
   char *lockname;
   int status;

public:
   BinFile(const char *filename, const char *mode, int withlock);
   ~BinFile();
   // returns if the binfile created in read mode is on a file that exists
   int exists();
   inline void swap2(char *);
   inline void swap4(char *);
   inline void swap8(char *);

   // Binary writing
   void write(char *i, int);
   BinFile& operator << (const double);
   BinFile& operator << (const float);
   BinFile& operator << (const void *);
   BinFile& operator << (const char *);
   BinFile& operator << (const int);
   BinFile& operator << (const unsigned int);
   BinFile& operator << (const unsigned long);
   BinFile& operator << (const unsigned short);
   BinFile& operator << (const signed long);
   BinFile& operator << (const signed short);
   BinFile& operator << (const signed char);
   BinFile& operator << (const unsigned char);
   BinFile& operator << (const char);

   // Binary reading
   void read(char *i, int);
   BinFile& operator >> (double &);
   BinFile& operator >> (float &);
   BinFile& operator >> (char *(&));
   BinFile& operator >> (void *(&));
   BinFile& operator >> (int &);
   BinFile& operator >> (unsigned int &);
   BinFile& operator >> (unsigned long &);
   BinFile& operator >> (unsigned short &);
   BinFile& operator >> (signed short &);
   BinFile& operator >> (signed long &);
   BinFile& operator >> (signed char &);
   BinFile& operator >> (unsigned char &);
   BinFile& operator >> (char &);

   void open(const char* filename, const char *mode);
   void close();
};

#endif
