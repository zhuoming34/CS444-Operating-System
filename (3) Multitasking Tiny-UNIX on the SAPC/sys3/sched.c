// filename: sched.c
// description: scheduler functions schedule(), sleep(), wakeup()
#include <cpu.h>
#include <stdio.h>
#include "sched.h"
#include "proc.h"

#define BUFLEN 20

// debug_log() for process switches
void debug_log_procswch(PEntry *preproc, int preIdx, int curIdx);

extern void asmswtch(PEntry *oldpentryp, PEntry *newpentryp);
//extern PEntry proctab[NPROC];
extern void debug_log(char *);
void print_proc(int idx);
void reset_proc(int idx);
extern void shutdown(void);
extern int N_zombie;

/* schedule():
* look for a user program to run, if not, run process 0.
* call asmswtch.
* called from process 0: from startup, sleep and sysexit
*/
void schedule(void) {
 	
	PEntry *preproc = curproc; // previous process's
	//char log[BUFLEN];	
	int i, preIdx, curIdx;
	
	int saved_eflags = get_eflags();
	cli();
	
	//for (i=0; i<NPROC; i++)
	//	print_proc(i);
	/* NPROC = 4 */
	for (i=0; i<NPROC; i++) {
		if (&proctab[i] == preproc) 
			preIdx = i;
	}
		
	for (i=1; i<=NPROC; i++) {
		//printf("%d+",i);
		/* for processes 1,2,3 */
		if (i<NPROC && proctab[i].p_status==RUN) {
			/* if one process can be ran, run it */
			curproc = &proctab[i];
			curIdx = i;
			break;
		}
		/* if i>3, all processes have been ran, so run proc0 */
		else if (i==NPROC) {
			curproc = &proctab[0];
			curIdx = 0;
			// a page fault at asmswtch after here
		}
	}
	//print_proc(preIdx);
	//if (curIdx != 0)
	//	printf("\npreIdx=%d, curIdx=%d\n", preIdx,curIdx);
	if (preIdx != curIdx) // avoid (0-0)'s
		debug_log_procswch(preproc, preIdx, curIdx);
	
	asmswtch(preproc, curproc); // switch processes
	//sti();
	set_eflags(saved_eflags);

	//return;
}

void print_proc(int idx){
	if (idx==0)
		printf("\n");
	printf("proc%d: ", idx);
	for (int j=0; j<7; j++) {
		printf("%d,",proctab[idx].p_savedregs[j]);
	}
	if (proctab[idx].p_status == RUN)
		printf("RUN,");
	if (proctab[idx].p_status == ZOMBIE)
		printf("ZOMBIE,");
	if (proctab[idx].p_status == BLOCKED)
		printf("BLOCKED,");
	if (proctab[idx].p_waitcode == TTY0_OUTPUT)
		printf("TTY0,");
	if (proctab[idx].p_waitcode == TTY1_OUTPUT)
		printf("TTY1,");
	printf("%d\n",proctab[idx].p_exitval);
}

void reset_proc(int idx){
	proctab[idx].p_savedregs[SAVED_ESP] = 0;
	proctab[idx].p_savedregs[SAVED_EBX] = 0;
	proctab[idx].p_savedregs[SAVED_ESI] = 0;
	proctab[idx].p_savedregs[SAVED_EDI] = 0;
	proctab[idx].p_savedregs[SAVED_EBP] = 0;
	proctab[idx].p_savedregs[SAVED_EFLAGS] = 0;
	proctab[idx].p_savedregs[SAVED_PC] = 0;
	proctab[idx].p_status = RUN;
	proctab[idx].p_waitcode = TTY1_OUTPUT;
	proctab[idx].p_exitval = 0;
}
/* sleep():
* block the calling process by changing p_status to BLOCKED,
* p_waitcode to to event and then calling schedule()
* called from ttywrite (to replace the busy wait) 
*/
void sleep(WaitCode event) {
	// char log[BUFLEN];
	//printf("sleep()\n");
	int saved_eflags = get_eflags();
	cli();
	curproc->p_status = BLOCKED;
	curproc->p_waitcode = event;
	schedule();
	//sti();
	set_eflags(saved_eflags);
	return;	
}


/* wakeup
* loop through proctab, changing all processes blocked
* on this event to p_status=RUN
* called from tty output interrupt handler
*/
void wakeup(WaitCode event) {
	// char log[BUFLEN];
	//printf("wakeup()\n");
	int saved_eflags = get_eflags();
	cli();
	for (int i=1; i<NPROC; i++) {
		if (proctab[i].p_waitcode == event && proctab[i].p_status == BLOCKED)
			proctab[i].p_status = RUN;
	}
	//sti();
	set_eflags(saved_eflags);
	return;
}


/* debug log for process switches 
* |(0 -1) switch from proc 0 to proc 1
* |(1Z-2) switch from proc 1, now a zombie, to proc 2
* |(1B-2) switch from proc 1, now blocked, to proc 2
*/
void debug_log_procswch(PEntry *preproc, int preIdx, int curIdx) {
	char log[BUFLEN];
	// printf("proc swch debug log\n");
	// check and store status of previous process
	if (preproc->p_status == ZOMBIE) {
		sprintf(log, "|(%dZ-%d)", preIdx, curIdx);
		//printf("\n");
		//printf(log);
	} else if (preproc->p_status == BLOCKED) {
		sprintf(log, "|(%dB-%d)", preIdx, curIdx);
		//printf("\n");
		//printf(log);
	} else {
		sprintf(log, "|(%d -%d)", preIdx, curIdx);
		//printf("\n");
		//printf(log);
	}
	debug_log(log);
}
