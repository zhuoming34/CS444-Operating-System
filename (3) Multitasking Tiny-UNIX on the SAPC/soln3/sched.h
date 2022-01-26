/*
 * Kernel user process scheduling routines.
 *
 * author: Zhihuan Weng
 */

#ifndef SCHED_H
#define SCHED_H

#include "proc.h"

/*
 * Context switch to a runnable user process. If no such
 * process exists, switch to kernel.
 *
 * Returns 0 if all user processes has terminited with 
 * a ZOMBIE state. Otherwise, return a non-zero integer.
 */
int schedule(void);


/*
 * Block the calling process and then call schedule()
 */
void sleep(WaitCode event);

/*
 * Wake up all user processes blocked on the given event
 * and mark them as runnable.
 *
 * event -- a code descrbing the resource that becomes
 *          available for other processes.
 */
void wakeup(WaitCode event);

#endif
