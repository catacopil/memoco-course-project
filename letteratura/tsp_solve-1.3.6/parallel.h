// A parallel computing module
//
// orig ceh 9-23-94

#ifndef _PARALLEL_H
#define _PARALLEL_H

#include "io.h"
#include "circlist.h"
#include "tour.h"

#define STARTED_STATUS    0x01
#define WAITING_STATUS    0x02
#define SUSPENDED_STATUS  0x04
#define PRIMARY_STATUS    0x80

// if priority is negative this means the TspHost is not being run yet,
// therefore a waiting host can delete the negative priority host and
// start to process its todo argument.  The primary host has a priority of 0.
class TspHost : public CircListElement {
public:
   char *hostname, *tsp_solve_path, *todo;
   signed long priority, pid, status, usr1sig, usr2sig, session;
   void write(Term &);
   TspHost(Term &file);
   void new_file(const char *);
   TspHost();
   ~TspHost();
   inline TspHost* get_next() { return (TspHost*)CircListElement::get_next(); };
   inline TspHost* get_prev() { return (TspHost*)CircListElement::get_prev(); };
};

class Parallel {
   int tmp_inc;
   long num_hosts;
   CircList hosts;
   char *hostfile_name;
   TspHost *myhost;
   signed long read_hosts(int);
   void write_hosts(int);
   void make_myhost();
   // this will free the files and the data structures if something in
   // parallel fails with in parallel, otherwise you can just destruct
   // Parallel with the destructor from the outside if you want files cleared.
   void die();
   inline TspHost *could_make_something_to_do(int);
public:
   Parallel();
   void init(); // only to be called once after Params is read
   ~Parallel();
   void destruct(int normal); // since destructors can't have parameters

   // looks at the current network and looks for something to do, if nothing
   // is found it returns 0.  If something was found, then it modifies and
   // overwrites the Param structure accordingly, so the main loop can process
   // the commandline instructions.
   //
   // If there is nothing left to do in the current hosts session, and the
   // current hosts is a primary client, then delete the current host files
   // that may have accumulated (namely the session's tour file.)
   int found_something_to_do();

   // puts the file "todo" in the host list for other clients to do.
   // the todo is usually the parameter returned by Parallel::tmpname(&todo)
   int make_something_to_do(const char *todo);

   // to be called before make_something_to_do(), if you want to be sure there
   // is something available before you actually make something to do.
   int could_make_something_to_do();

   // if a new better tour is found then update it to the tour file of the
   // current session.  if tell_others is zero then don't tell anyone else
   // you've found a better tour.
   void update_new_tour(Tour *& tour, sum_t &sum, int tell_others,
      const Matrix *);

   // returns non-zero if this is the primary client and all other clients on
   // this session are done running.
   //
   // Important: Assumes you are calling this when the current algorithm is
   // finished, waiting and about to call found_something_to_do() to find
   // something more to do.
   int my_session_done();

   // to be called when the process is suspended
   void suspend_process();
   // to be called when the process is continued
   void continue_process();

   // call to find a temporary file name in the nfsdir
   char *tmpname(char **with_out_full_path);

   // If the tmpname returned by tmpname() when passed as to
   // make_something_to_do() fails, then this should be called to remove the
   // tempfile and to reset the file counter, to minimied the numbers used for
   // tempfilenames
   void undo_tmpname(char *tempname);
   void remove_tmpname(char *tempname);

   // ends process of saving this process by deleting the old filename and
   // changing the hosts file to read the new filename.
   void make_new_filename(const char *filename, const char *wopath);
};

extern Parallel parallel;

#endif
