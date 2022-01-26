// filename: sched.h
// prototypes for scheduler, schedule(), sleep(), wakeup()

#ifndef SCHED_H
#define SCHED_H 

#include "proc.h" // defines WaitCode

void schedule(void);
void sleep(WaitCode event);
void wakeup(WaitCode event);

#endif

