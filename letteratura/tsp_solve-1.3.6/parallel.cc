// A parallel computing module
//
// orig ceh 9-23-94

#include <assert.h>
#include "parallel.h"
#include "params.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <signal.h>

#define FINDSLEEP 2

Parallel parallel;

#ifndef HAS_GETHOSTNAME_DECL
extern "C" {
extern int gethostname(char *, int);
};
#endif

#define MAXBUF 200

char *Parallel :: tmpname(char **wopath)
{
   char buf[MAXBUF], *str;

   make_myhost();
   if (myhost != NULL) {
      sprintf(buf, "%s%d%2.2s-%d.tsp", (param.nfsdir == NULL)
       ? "" : param.nfsdir, (int)myhost->priority, myhost->hostname,
       (int)tmp_inc++);
      str = strduup(buf, 0);
      for ((*wopath)=str+strlen(str); *(*wopath) != '/'; (*wopath)--)
         ;
      (*wopath)++;
   }
   else
      *wopath = str = strduup("", 0);
   return str;
}

void Parallel :: remove_tmpname(char *tmpname)
{
   unlink(tmpname);
}

void Parallel :: undo_tmpname(char *tmpname)
{
   this->remove_tmpname(tmpname);
   tmp_inc--;
}

#ifdef HAS_NFSLOCK

inline TspHost *Parallel :: could_make_something_to_do(int i)
{
   TspHost *host, *newhost = NULL;
   int waiting_hosts, todo_hosts;
   CircListIter ci;

   waiting_hosts = todo_hosts = 0;
   read_hosts(i);
   for (ci.init(hosts); (host = (TspHost*)ci.next()) != NULL; ) {
      if ((host->status & WAITING_STATUS) && host->priority >= 0
       && host != myhost) {
         newhost = host;
         waiting_hosts++;
      }
      if (host->priority < 0)
         todo_hosts++;
   }
   return (waiting_hosts > todo_hosts) ? newhost : ((TspHost*)NULL);
}

int Parallel :: could_make_something_to_do()
{
   return (int)could_make_something_to_do(1);
}

// Point *1*: only split the work if there is someone waiting for it
int Parallel :: make_something_to_do(const char *todo)
{
   TspHost *newhost;
   char *lockname;
   int ret = 0;

   if (!param.invoked || param.nfsdir == NULL)
      return 0;
   make_myhost();
   if ((lockname = lockfile(hostfile_name, myhost->priority)) != NULL) {
      newhost = could_make_something_to_do(0);
      if (newhost == NULL) { /*1*/
         ret = 0;
      }
      else {
         newhost = new TspHost();
         newhost->priority = read_hosts(0);
         assert(newhost->priority != 0);
         newhost->session = myhost->session;
         newhost->new_file(todo);
         dump << newhost->todo << "$$$$$$$$\n";
         hosts.insert(newhost);
         num_hosts++;
         write_hosts(0);
         ret = 1;
      }
   }
   unlockfile(lockname);
   if (ret) {
      dump << "*Created " << todo << "\n";
      dump.flush();
   }
   return ret;
}

// Point *1*: The session is now done, if the primary client is waiting
// (since it is calling this function) and there are no more running
// clients on the primary clients session.
//
int Parallel :: my_session_done()
{
   TspHost *p;
   int done;
   CircListIter ci;

   if (!param.invoked || param.nfsdir == NULL)
      return 1;
   make_myhost();
   if (!(myhost->status & PRIMARY_STATUS))
      return 0;
   read_hosts(1);
   for (ci.init(hosts); (p = (TspHost*)ci.next()) != NULL; ) {
      if (p != myhost && !(p->status & WAITING_STATUS)
       && p->session == myhost->session)
         break;
   }
   if ((done = (p == NULL))) { /*1*/
      char buf[MAXBUF];
      sprintf(buf, "%s%dtour.tsp", param.nfsdir, (int)myhost->session);
      unlink(buf);
   }
   return done;
}

// Point *1*: since no client is running it is possible to return normally
//
// Point *2*: try to find a running primary client with the same session id 
//
// Point *3*: Includes myhost just incase i'm still running from a suspend
//
// Point *4*: To ReOrder the bunch so that other clients will get the chance
// to get signaled, if some are too slow or aren't able to respond to signals
//
// Point *5*: Only exit when all clients are waiting for something to do and
// there are no things todo (things with negative priority)
//
int Parallel :: found_something_to_do()
{
   TspHost *p, *psession, *newhead;
   int ret, all_waiting;
   char *lockname, buf[MAXBUF];
   CircListIter ci;

   make_myhost();
   if (!param.invoked || param.nfsdir == NULL
    || (lockname = lockfile(hostfile_name, myhost->priority)) == NULL)
      return 0;
   psession = newhead = NULL;
   myhost->status |= WAITING_STATUS;
   ret = 0;
   for (read_hosts(0), write_hosts(0); (all_waiting = 1); read_hosts(0)) {
      for (ci.init(hosts); (p = (TspHost*)ci.next()) != NULL; ) {
         newhead = p;
         if (p->priority < 0)
            psession = p;
         if (!(p->status & WAITING_STATUS)) /*3*/
            all_waiting = 0;
      }
      hosts.change_head_to(newhead);
      write_hosts(0); /*4*/
      if (all_waiting && psession != NULL) /*5*/
         break;
      p = psession;
      if (p != NULL) {
dump << "Found This [" << p->todo << "]";
dump.Time();
dump.flush();
         psession = p;
         for (ci.init(hosts); (p = (TspHost*)ci.next()) != NULL; ) {
            if (p != myhost && p != psession && p->session == psession->session
             && p->priority >= 0 && (p->status & PRIMARY_STATUS)) /*2*/
               break;
         }
         if (p == NULL)
            myhost->status |= PRIMARY_STATUS;
         else
            myhost->status &= ~(PRIMARY_STATUS);
         p = psession;
         param.init(p->todo);
         if (myhost->todo != NULL)
            delete myhost->todo;
         myhost->todo = strduup(p->todo, 0);
         myhost->status &= ~(WAITING_STATUS);
         myhost->session = p->session;
         hosts.del(p);
         delete p;
         num_hosts--;
         write_hosts(0);
         ret = 1;
         break;
      }
      for (ci.init(hosts); (p = (TspHost*)ci.next()) != NULL; ) {
         if (p != myhost && p->priority >= 0 && !(p->status&WAITING_STATUS))
            break;
      }
      unlockfile(lockname);
      lockname = NULL;
      if (p == NULL) { /*1*/
         dump << "Could Not Find Anything Todo!\n";
         ret = 0;
         break;
      }
      else {
         dump << "Signaled " << p->hostname << "\n";
         dump.flush();
         sprintf(buf, "%s %s -n 'kill -%ld %ld'",
            REMOTE_COMMAND_SHELL, p->hostname, p->usr2sig, p->pid);
         system(buf);
      }
      sleep(FINDSLEEP);
      if ((lockname = lockfile(hostfile_name, myhost->priority)) == NULL) {
         ret = 0;
         break;
      }
   }
   unlockfile(lockname);
   return ret;
}

signed long Parallel :: read_hosts(int withlock)
{
   int ret, do_add;
   TspHost *p;
   Term file(0);
   char *lockname;
   signed long priority, lowpriority, session;
   CircListIter ci;

   priority = lowpriority = session = 0;
   make_myhost();
   lockname = (withlock
    ? lockfile(hostfile_name, myhost->priority) : ((char*)NULL));
   assert(withlock ? (lockname != NULL) : 1);
   if ((ret=file.open(hostfile_name, "r", 0)) && ret != 3) {
      dump << "Can't Open for read " << hostfile_name << "\n";
      unlockfile(lockname);
      die();
   }
   else if (ret == 3)
      num_hosts = 0;
   else
      file >> num_hosts;
   for (ci.init(hosts); (p = (TspHost*)ci.next()) != NULL; ) {
      hosts.del(p);
      if (p != myhost)
         delete p;
   }
   for (do_add = 1, ret = 0; ret < num_hosts; ret++) {
      p = new TspHost(file);
      if (p->pid == myhost->pid && !strcmp(p->hostname, myhost->hostname)) {
         delete p;
         do_add = 0;
         p = myhost;
      }
      hosts.insert(p);
      if (priority < p->priority)
         priority = p->priority;
      if (lowpriority > p->priority)
         lowpriority = p->priority;
      if (session < p->session)
         session = p->session;
   }
   file.close();
   assert(myhost != NULL);
   if (myhost->status & STARTED_STATUS) {
      myhost->status &= (~STARTED_STATUS);
      if (myhost->status & PRIMARY_STATUS)
         myhost->session = session+1;
   }
   if (do_add) {
      hosts.insert(myhost);
      if (lockname != NULL || !withlock) {
         myhost->priority = priority+1;
         num_hosts++;
         write_hosts(0);
      }
   }
   unlockfile(lockname);
   return lowpriority-1;
}

void Parallel :: write_hosts(int withlock)
{
   TspHost *p;
   Term file(0);
   char *lockname;
   CircListIter ci;

   if (withlock) {
      make_myhost();
      lockname = lockfile(hostfile_name, myhost->priority);
      assert(lockname != NULL);
      read_hosts(0);
   }
   else
      lockname = NULL;
   if (file.open(hostfile_name, "w", 0) == 2) {
      dump << "Can't Open for write " << hostfile_name << "\n";
      dump.flush();
      unlockfile(lockname);
      die();
   }
   file << num_hosts << "\n";
   for (ci.init(hosts); (p = (TspHost*)ci.next()) != NULL; ) {
      p->write(file);
   }
   file.close();
   unlockfile(lockname);
}

void Parallel :: make_myhost()
{
   char buf[MAXBUF], *tmp;

   if (myhost != NULL || !param.invoked || param.nfsdir == NULL)
      return;
   myhost = new TspHost();
   myhost->pid = getpid();
   myhost->usr1sig = SIGUSR1;
   myhost->usr2sig = SIGUSR2;
   myhost->status = ((param.num_abrevs>0) ? PRIMARY_STATUS : 0)|STARTED_STATUS;
   if (gethostname(buf, MAXBUF)==-1) {
      dump << "Could Not Get Host name.\n";
      die();
   }
   myhost->hostname = strduup(buf, 0);
   if (*param.progname == '/')
      strcpy(buf, param.progname);
   else  {
      FILE *pwd = popen("pwd", "r");
      fgets(buf, MAXBUF, pwd);
      pclose(pwd);
      tmp = buf + strlen(buf)-2;
      if (*tmp != '/') {
         tmp[1] = '/';
         tmp[2] = 0;
      }
      strcat(buf, param.progname);
   }
   myhost->tsp_solve_path = strduup(buf, 0);
}

Parallel :: Parallel()
{
   myhost = NULL;
   hostfile_name = NULL;
   tmp_inc = 0;
}

void Parallel :: init()
{
   char buf[MAXBUF];

   if (!param.invoked || param.nfsdir == NULL)
      return;
   sprintf(buf, "%shosts", param.nfsdir);
   hostfile_name = strduup(buf, 0);
   read_hosts(1);
}

void Parallel :: die()
{
   destruct(0);
   assert(0);
}

Parallel :: ~Parallel(int normal)
{
   TspHost *p;
   CircListIter ci;

   myhost = NULL;
   if (hostfile_name != NULL) {
      delete hostfile_name;
      hostfile_name = NULL;
   }
   for (ci.init(hosts); (p = (TspHost*)ci.next()) != NULL; ) {
      hosts.del(p);
      delete p;
   }
}

/* Point *1*: the Parallel is being destructed abnormally,
 * so i'll need to leave it in the todo list with negative priority to be done
 * again since this process is crashing.
 *
 * Point *3*: if the current host has no todo list, then the whole session
 * is sabatoged, and delete the rest of the same session.  there is a problem
 * with this because the remainder of clients of the same session should be
 * signaled to stop, or they will keep going and read them selves to the
 * host file.
 *
 * Point *3*: addendum: it doesn't seem like you want to sabatage the whole
 * session, but it does this only when the current host has no todo list,
 * which meant the info being calcuated here would not be savable so why
 * continue the rest of the session, when part of it is going to be missed.
 */
void Parallel :: destruct(int normal)
{
   char *lockname;
   CircListIter ci;

   dump.flush();
   param.recover_bits &= ~RECOVERFINISHED;
   if (myhost != NULL) {
      if (param.invoked && param.nfsdir != NULL) {
         lockname = lockfile(hostfile_name, myhost->priority);
         assert(lockname != NULL);
         myhost->priority = read_hosts(0); /*1*/
         myhost->status |= WAITING_STATUS;
         if (normal || myhost->todo == NULL) {
            if (myhost->todo == NULL) {
               TspHost *p;
               for (ci.init(hosts); (p = (TspHost*)ci.next()) != NULL; ) {
                  if (p != myhost && p->session == myhost->session) { /*3*/
                     hosts.del(p);
                     delete p;
                     num_hosts--;
                  }
               }
            }
            hosts.del(myhost);
            delete myhost;
            num_hosts--;
         }
         write_hosts(0);
         unlockfile(lockname);
      }
      myhost = NULL;
   }
   this->Parallel::~Parallel();
}

// Point *1*: Not only to close the file but unlock it for further use
void Parallel :: update_new_tour(Tour *& tour, sum_t &sum, int tell_others,
   const Matrix *m)
{
   char buf[MAXBUF];
   sum_t oldsum;
   Tour oldtour;
   CircListIter ci;

   if (!param.invoked || param.nfsdir == NULL)
      return;
   sprintf(buf, "%s%dtour.tsp", param.nfsdir, (int)myhost->session);
   Term oldtourfile(0);
   make_myhost();
   assert(myhost->priority != 0);
   if (!oldtourfile.open(buf, "r", myhost->priority)) {
      oldtourfile >> oldsum;
      oldtour.read(oldtourfile);
      if (!oldtour.is_complete())
         oldsum = MAX_SUM;
      else
         oldsum = oldtour.cost(m);
   }
   else
      oldsum = MAX_SUM;
   oldtourfile.Term::~Term(); /*1*/

   if (oldsum > sum && tell_others) {
      TspHost *p;
      Term tourfile(0);
      tourfile.open(buf, "w", myhost->priority);
      tourfile << sum;
      tour->write(tourfile);
      tourfile.Term::~Term(); /*1*/
      read_hosts(1);
      for (ci.init(hosts); (p = (TspHost*)ci.next()) != NULL; ) {
         if (p != myhost && p->priority >= 0 && !(p->status&WAITING_STATUS)) {
            sprintf(buf, "%s %s -n 'kill -%ld %ld'",
               REMOTE_COMMAND_SHELL, p->hostname, p->usr2sig, p->pid);
            system(buf);
         }
      }
   }
   else if (sum > oldsum) {
      tour->copy(oldtour);
dump << "New Tour " << oldsum << ", From Tour " << sum << "\n";
dump.flush();
      sum = oldsum;
   }
}

void Parallel :: suspend_process()
{
   char *lockname;

   if (myhost != NULL && !(myhost->status & WAITING_STATUS) && param.invoked
    && !(myhost->status & SUSPENDED_STATUS) && param.nfsdir != NULL) {
      dump.Time();
      dump << "Client Suspended\n";
      dump.flush();
      lockname = lockfile(hostfile_name, myhost->priority);
      read_hosts(0);
      myhost->status |= WAITING_STATUS|SUSPENDED_STATUS;
      write_hosts(0);
      unlockfile(lockname);
   }
}

void Parallel :: continue_process()
{
   char *lockname;

   make_myhost();
   if (myhost != NULL && param.invoked && param.nfsdir != NULL
    && (myhost->status & SUSPENDED_STATUS)) {
      lockname = lockfile(hostfile_name, myhost->priority);
      read_hosts(0);
      dump.Time();
      dump << "Client Continued\n";
      dump.flush();
      myhost->status &= ~(WAITING_STATUS|SUSPENDED_STATUS);
      write_hosts(0);
      unlockfile(lockname);
   }
}

// ends process of saving this process by deleting the old filename and
// changing the hosts file to read the new filename.
void Parallel :: make_new_filename(const char *filename, const char *wopath)
{
   char *lockname;

   if (param.recover_ptr != NULL) {
      delete param.recover_ptr;
      param.recover_ptr = NULL;
   }
   if (param.recover_file != NULL) {
      if (param.nfsdir != NULL) {
         dump << "*Deleteing " << param.recover_file << "\n";
         remove_tmpname(param.recover_file);
      }
      delete param.recover_file;
      param.recover_file = NULL;
   }
   param.recover_ptr = strduup(wopath, 0);
   param.recover_file = strduup(filename, 0);
   make_myhost();
   lockname = lockfile(hostfile_name, myhost->priority);
   read_hosts(0);
   myhost->new_file(wopath);
   write_hosts(0);
   unlockfile(lockname);
}

#else

int Parallel :: could_make_something_to_do() { return 0; }
void Parallel :: make_myhost() {}
int Parallel :: my_session_done() { return 1; }
Parallel :: Parallel() {}
void Parallel :: init() {}
void Parallel :: destruct(int) {}
Parallel :: ~Parallel() {}
void Parallel :: write_hosts(int withlock) {}
signed long Parallel :: read_hosts(int) { return 0; }
int Parallel :: found_something_to_do() { return 0; }
int Parallel :: make_something_to_do(const char *) { return 0; }
void Parallel :: update_new_tour(Tour *& tour, sum_t &sum, int) {}
void Parallel :: suspend_process() {}
void Parallel :: continue_process() {}
void Parallel :: make_new_filename(char *) {}

#endif // NFSLOCK

TspHost :: TspHost(Term &file) : CircListElement()
{
   hostname = tsp_solve_path = todo = NULL;
   file >> session >> priority >> status >> pid >> usr1sig >> usr2sig
      >> hostname >> todo >> tsp_solve_path;
}

void TspHost :: write(Term &file)
{
   file.form("%-3ld%-3ld%4ld%6ld%3ld%3ld ",
      session, priority, status, pid, usr1sig, usr2sig);
   file << hostname << " " << todo << " " << tsp_solve_path << "\n";
}

TspHost :: TspHost() : CircListElement()
{
   hostname = tsp_solve_path = todo = NULL;
   priority = pid = status = usr1sig = usr2sig = 0;
   session = -1;
}

TspHost :: ~TspHost()
{
   if (hostname != NULL)
      delete hostname;
   if (tsp_solve_path != NULL)
      delete tsp_solve_path;
   if (todo != NULL)
      delete todo;
}

void TspHost :: new_file(const char *file)
{
   char *tmp, b0, b1, b2;

   if (todo != NULL)
      delete todo;
   tmp = todo = strduup(file, 2);
   b0 = tmp[0];
   b1 = tmp[1];
   b2 = tmp[2];
   tmp[0] = '-';
   tmp[1] = 'r';
   while (b0) {
      tmp[2] = b0;
      b0 = b1;
      b1 = b2;
      tmp++;
      b2 = tmp[2];
   }
   tmp[2] = b0;
}

