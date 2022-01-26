#include <stdio.h>
#include <cpu.h>
#include <pic.h>
#include <gates.h>
#include "tsystm.h"
#include "tsyscall.h"
#include "tunistd.h"


//extern void syscall(void);
extern IntHandler _syscall;
extern void _ustart(void);
extern void _shutdown(void);
void init_kernel(void);
void syscallc(int user_eax, int arg1, char *buf, int nchar);
int sysexit(int exitcode);


void init_kernel() {
	ioinit();
	set_trap_gate(0x80, &_syscall); // transfer control to kernel
	// main(); // later should call ustart();
	_ustart();
}



/* switch over various different syscalls */
void syscallc(int user_eax, int arg1, char *buf, int nchar) {
	switch(user_eax) {
		// syscall num = 1
		case SYS_EXIT:
			kprintf("calling SYSEXIT\n");
			// arg1 = exitcode
			user_eax = sysexit(arg1);
			break;
		// syscall num = 3
		case SYS_READ:
			kprintf("calling SYSREAD\n");
			// arg1 = dev
			user_eax = sysread(arg1, buf, nchar);
			break;
		// syscall num = 4
		case SYS_WRITE:
			kprintf("calling SYSWRITE\n");
			// arg1 = dev
			user_eax = syswrite(arg1, buf, nchar);
			break;
		default:
			kprintf("system call not available");
	}
}


/* syscall exit */
int sysexit(int exitcode) {
	kprintf("\nexit code -> %d\n", exitcode);
	kprintf("shutting down\n");
	_shutdown();
	return 0;
}
