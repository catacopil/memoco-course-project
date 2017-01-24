/* binfile.cc
 *
 * Binary file utility
 *
 * orig ceh
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "binfile.h"
#include "io.h"
#include "chlib.h"
#include "stdmacro.h" // only for die_if()

#define BINFILE_EXISTS 0x1

#ifndef WORDS_BIGENDIAN

inline void BinFile::swap2(char *two_bytes)
{
   char swap;
   swap = two_bytes[0];
   two_bytes[0] = two_bytes[1];
   two_bytes[1] = swap;
}

inline void BinFile::swap4(char *four_bytes)
{
   swap2(four_bytes+1);
   char swap;
   swap = four_bytes[0];
   four_bytes[0] = four_bytes[3];
   four_bytes[3] = swap;
}

inline void BinFile::swap8(char *eight_bytes)
{
   char swap;
   swap = eight_bytes[0];
   eight_bytes[0] = eight_bytes[7];
   eight_bytes[7] = swap;
   swap = eight_bytes[1];
   eight_bytes[1] = eight_bytes[6];
   eight_bytes[6] = swap;
   swap4(eight_bytes+2);
}

#else

inline void BinFile::swap2(char *)
{
}

inline void BinFile::swap4(char *)
{
}

inline void BinFile::swap8(char *)
{
}

#endif

inline void check_file(void* fp)
{
   die_if(fp == NULL, "BinFile Not Properly Initialized", "");
}

void BinFile :: read(char *in, int size)
{
   check_file(fp);
   fread(in, size, 1, (FILE*)fp);
   switch (size) {
   case 1: break;
   case 4: swap4(in); break;
   case 8: swap8(in); break;
   case 2: swap2(in); break;
   default:
      dump << "Can't use that type of binary input.\n";
      dump.flush();
      assert(0);
   }
}

void BinFile :: write(char *in, int size)
{
   check_file(fp);
   switch (size) {
   case 1: break;
   case 4: swap4(in); break;
   case 8: swap8(in); break;
   case 2: swap2(in); break;
   default:
      dump << "Can't use that type of binary output.\n";
      dump.flush();
      assert(0);
   }
   fwrite(in, size, 1, (FILE*)fp);
}

/* Point *1*: each time a binfile is opened, we check the validity of the
 * binary file with the current machines archtecture from the sizeof_array,
 * which contains sizeof() values for standard base data types which may
 * be hazardously run if different than when stored.
 */
#define MAXFILENAMELEN 100
BinFile::BinFile(const char *filename, const char *mode, int withlock)
{
   if (withlock)
      lockname = lockfile(filename, 0);
   else
      lockname = NULL;
   fp = (void*)fopen(filename, mode);
   if (fp == NULL) {
      unlockfile(lockname);
      lockname = NULL;
      status = 0;
   }
   else {
      status = BINFILE_EXISTS;
      float *so = sizeof_array, cmp;
      if (strstr(mode, "r")) { /*1*/
         do {
#ifdef OLD_TIMER
if (*so != 1.)
{
#endif
            *this >> cmp;
            die_if(cmp != *so, "Reading file saved using different types", "") ;
#ifdef OLD_TIMER
}
#endif
         } while (*++so != 0);
      }
      else {
         assert(strstr(mode, "w"));
         do {
            *this << *so;
         } while (*++so != 0);
      }
   }
}

BinFile::~BinFile()
{
   unlockfile(lockname);
   lockname = NULL;
   close();
}

void BinFile::close()
{
   if (fp != NULL) {
      fclose((FILE*)fp);
      fp = NULL;
   }
}

BinFile& BinFile :: operator << (const double in)
{
   write((char*)&in, 8);
   return *this;
}

BinFile& BinFile :: operator << (const float in)
{
   write((char*)&in, 4);
   return *this;
}

/*
BinFile& BinFile :: operator << (const char *in)
{
   _uchar *str = (_uchar*)in;
   do {
      write((char*)str, 1);
   } while (*str++ != 0);
   return *this;
}
 */

BinFile& BinFile :: operator << (const void *in)
{
   write((char*)&in, 4);
   return *this;
}

BinFile& BinFile :: operator << (const char *in)
{
   const char *str = in;

   if (str == NULL) {
      *this << "^";
   }
   else {
      do {
         write((char*)str, 1);
      } while (*str++ != 0); // '\0'
   }
   return *this;
}

BinFile& BinFile :: operator << (const int in)
{
   write((char*)&in, sizeof(int));
   return *this;
}

BinFile& BinFile :: operator << (const unsigned int in)
{
   write((char*)&in, sizeof(unsigned int));
   return *this;
}

BinFile& BinFile :: operator << (const unsigned long in)
{
   write((char*)&in, 4);
   return *this;
}

BinFile& BinFile :: operator << (const unsigned short in)
{
   write((char*)&in, 2);
   return *this;
}

BinFile& BinFile :: operator << (const signed long in)
{
   write((char*)&in, 4);
   return *this;
}

BinFile& BinFile :: operator << (const signed short in)
{
   write((char*)&in, 2);
   return *this;
}

BinFile& BinFile :: operator << (const unsigned char in)
{
   write((char*)&in, 1);
   return *this;
}

BinFile& BinFile :: operator << (const signed char in)
{
   write((char*)&in, 1);
   return *this;
}

BinFile& BinFile :: operator << (const char in)
{
   write((char*)&in, 1);
   return *this;
}

BinFile& BinFile :: operator >> (double &in)
{
   read((char*)&in, 8);
   return *this;
}

BinFile& BinFile:: operator >> (float &in)
{
   read((char*)&in, 4);
   return *this;
}

BinFile& BinFile :: operator >> (void *(&in))
{
   read((char*)&in, 4);
   return *this;
}

BinFile& BinFile :: operator >> (char *(&in))
{
   char buf[MAXBINSTRING+1], *str;

   if (in != NULL)
      delete in;
   for (str = buf; ; str++) {
      read((char*)str, 1);
      if (*str == 0)
         break;
      die_if((str-buf) > MAXBINSTRING, "Too Large a character string", "");
   }
   if (buf[0] == '^' && buf[1] == 0) { // '\0'
      in = NULL;
   }
   else {
      in = new char[(int)(str-buf)+1];
      memcpy(in, buf, (int)(str-buf)+1);
   }
   return *this;
}

BinFile& BinFile :: operator >> (int &in)
{
   read((char*)&in, sizeof(int));
   return *this;
}

BinFile& BinFile :: operator >> (unsigned int &in)
{
   read((char*)&in, sizeof(unsigned int));
   return *this;
}

BinFile& BinFile :: operator >> (unsigned long &in)
{
   read((char*)&in, 4);
   return *this;
}

BinFile& BinFile :: operator >> (unsigned short &in)
{
   read((char*)&in, 2);
   return *this;
}

BinFile& BinFile :: operator >> (signed long &in)
{
   read((char*)&in, 4);
   return *this;
}

BinFile& BinFile :: operator >> (signed short &in)
{
   read((char*)&in, 2);
   return *this;
}

BinFile& BinFile :: operator >> (unsigned char &in)
{
   read((char*)&in, 1);
   return *this;
}

BinFile& BinFile :: operator >> (signed char &in)
{
   read((char*)&in, 1);
   return *this;
}
BinFile& BinFile :: operator >> (char &in)
{
   read((char*)&in, 1);
   return *this;
}

int BinFile :: exists()
{
   return status&BINFILE_EXISTS;
}
