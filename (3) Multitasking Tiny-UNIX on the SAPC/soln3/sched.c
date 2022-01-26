/*
 * Kernel user process scheduling routines.
 *
 * author: Zhihuan Weng
 * modified:  Ron Cheung 4/15/21
 */

#include <stdio.h>
#include <cpu.h>
#include "sched.h"
#include "proc.h"

extern int asmswtch(PEntry *p1, PEntry *p2);
extern void shutdown(void);
extern void debug_log(char *);

/* prototype for private functions */
void swtchwrap(PEntry *p1, PEntry *p2);


/*
 * Context switch to a runnable user process. If no such
 * process exists, switch to kernel.
 *
 * Returns 0 if all user processes has terminited with
 * a ZOMBIE state. Otherwise, return a non-zero integer.
 */
int schedule(void)
{
  int i;
  int nextproc;       /* index to the next process to run */
  int zcount;         /* number of ZOMBIE processes */
  int saved_eflags;
  PEntry *proc;       

  nextproc = 0;
  zcount = 0;
  saved_eflags = get_eflags();
  cli();

 /* Check if all processes = zoombie */
 for (i = 1; i < NPROC; i ++) {
    proc = proctab + i;
    if(proc -> p_status == ZOMBIE){
       zcount ++;
    }
  }


/* perform round-robin scheduler
   0-1, 1-2, 2-3, 3-0
*/
  nextproc = curproc -proctab;
  for (i = (nextproc  +1) % NPROC; i != nextproc; i = (i+1) %NPROC){
   if(proctab[i].p_status == RUN) break;
  }

//if (i == nextproc){
//  return 0;
//zcount = NPROC -1;
//}
 /* adjust the global curproc  before context switch */
  proc = curproc;
  curproc = proctab + i;
  
  swtchwrap(proc, curproc); 
  
  set_eflags(saved_eflags);

  if(zcount == NPROC-1) {
    return 0;
  }

  return 1;
}

/*
 * Block the calling process and then call schedule()
 */

void sleep(WaitCode event){
   int saved_eflags;

   saved_eflags = get_eflags();
   cli();
   curproc -> p_status = BLOCKED;  
   curproc -> p_waitcode = event;
   schedule();
   set_eflags(saved_eflags);
}


/*
 * Wake up all user processes blocked on the given event
 * and mark them as runnable.
 *
 * event -- a code descrbing the resource that becomes
 *          available for other processes.
 */
void wakeup(WaitCode event){
  PEntry *prc;
  int i;
  int saved_eflags;

  saved_eflags = get_eflags();
  cli();
  for (i = 1; i < NPROC; i ++) {
    prc = proctab + i;
    if(prc -> p_status == BLOCKED && prc -> p_waitcode == event){
      prc -> p_status = RUN;
    }
  }
  set_eflags(saved_eflags);
}

/*
 * private function for generating debug output before performing 
 * the context switch
 */
void swtchwrap(PEntry * old, PEntry * new) {
   int index;
   char c;

   /* if the curent process is running (kernel proc), no need
    * to switch */
   if(old == new){
      return;
   }

   index = (old - proctab);
   switch(index){
     case 0: c = '0'; break;
     case 1: c = '1'; break;
     case 2: c = '2'; break;
     case 3: c = '3'; break;
     default: c = '?'; break;
   }

   debug_log("|(");
   debug_log(&c);
   if(old -> p_status == BLOCKED) {
      debug_log("b");
   } else if (old -> p_status == ZOMBIE) {
      debug_log("z");
   }

   index = (new - proctab);
   switch(index){
     case 0: c = '0'; break;
     case 1: c = '1'; break;
     case 2: c = '2'; break;
     case 3: c = '3'; break;
     default: c = '?'; break;
   }

   debug_log("-");
   debug_log(&c);
   debug_log(")");
  
   /* do the magic */ 
   asmswtch(old, new);
   return;
}

