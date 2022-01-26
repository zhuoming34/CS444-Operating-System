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


/* as provided in $pclibsrc/cpu.c */
/* non-debug version */
void set_trap_gate(int n, IntHandler *inthand_addr) {
	debug_set_trap_gate(n, inthand_addr, 0);
}


/* write the nth idt descriptor as a trap gate to inthand_addr */
void debug_set_trap_gate(int n, IntHandler *inthand_addr, int debug)
{
	char *idt_addr;
	Gate_descriptor *idt, *desc;
	unsigned int limit = 0;

	if (debug)
		kprintf("Calling locate_idt to do sidt instruction...\n");
	locate_idt(&limit,&idt_addr);
	/* convert to CS seg offset, i.e., ordinary address, then to typed pointer */
	idt = (Gate_descriptor *)(idt_addr - KERNEL_BASE_LA);
    	if (debug)
	        kprintf("Found idt at %x, lim %x\n",idt, limit);
      	desc = &idt[n];               /* select nth descriptor in idt table */
        /* fill in descriptor */
        if (debug)
		kprintf("Filling in desc at %x with addr %x\n",(unsigned int)desc, (unsigned int)inthand_addr);
	
	desc->selector = KERNEL_CS;   /* CS seg selector for int. handler */
	desc->addr_hi = ((unsigned int)inthand_addr)>>16; /* CS seg offset of inthand */
	desc->addr_lo = ((unsigned int)inthand_addr)&0xffff;
	desc->flags = GATE_P|GATE_DPL_KERNEL|GATE_TRAPGATE; /* valid, trap */
	desc->zero = 0;
}


/* switch over various different syscalls */
void syscallc(int user_eax, int arg1, char *buf, int nchar) {
	switch(user_eax) {
		// syscall num = 1
		case SYS_EXIT:
			// arg1 = exitcode
			user_eax = sysexit(arg1);
			break;
		// syscall num = 3
		case SYS_READ:
			// arg1 = dev
			user_eax = sysread(arg1, buf, nchar);
			break;
		// syscall num = 4
		case SYS_WRITE:
			// arg1 = dev
			user_eax = syswrite(arg1, buf, nchar);
			break;
		default:
			kprintf("not available");
	}
}


/* syscall exit */
int sysexit(int exitcode) {
	kprintf("\nexit code -> %d\n", exitcode);
	kprintf("shutting down\n");
	_shutdown();
	return 0;
}
