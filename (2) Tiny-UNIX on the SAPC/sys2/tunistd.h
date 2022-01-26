// tunistd.h
// prototypes for user mode system calls services
// similar to io_public.h, but exit instead of ioinit

#ifndef TUNISTD_H
#define TUNISTD_H

#include "tty_public.h"

// exit a program
int exit(int exitcode); 
// read nchar bytes into buf from dev
int read(int dev, char *buf, int nchar); 
// write nchar bytes from buf to dev
int write(int dev, char *buf, int nchar);
// misc. device func.
int control(int dev, int fncode, int val);

#endif
