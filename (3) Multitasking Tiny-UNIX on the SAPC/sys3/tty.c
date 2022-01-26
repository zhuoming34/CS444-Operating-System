/*********************************************************************
*
*       file:           tty.c
*       author:         betty o'neil
*                       Ray Zhang
*
*       tty driver--device-specific routines for ttys 
*
*       Implement the alternative approach of using the IIR 
*       acknowledgement method.
*/
#include <stdio.h>  /* for kprintf prototype */
#include <serial.h>
#include <cpu.h>
#include <pic.h>
#include "ioconf.h"
#include "tty_public.h"
#include "tty.h"
#include "queue/queue.h"
#include "proc.h"
#include "sched.h"

/* define maximum size of queue */
#define QMAX 6
#define BUFLEN 20
#define DEBUG_AREA 0x300000


//char *debug_log_area = (char *) DEBUG_AREA;
//char *debug_record;
//extern int kprintf(char* fmt, ...);
/* tell C about the assembler shell routines */
extern void irq3inthand(void), irq4inthand(void); 
/* C interrupt handlers called from assembler routines */
void irq4inthandc(void);
void irq3inthandc(void);
/* common code for interrupt handlers */
void irqinthandc(int);


extern void debug_log(char *);
struct tty ttytab[NTTYS];        /* software params/data for each SLU dev */

/*====================================================================
*
*       tty specific initialization routine for COM devices
*/

void ttyinit(int dev)
{
    int baseport;
    struct tty *tty;		/* ptr to tty software params/data block */
    //debug_record =debug_log_area;

    baseport = devtab[dev].dvbaseport; /* pick up hardware addr */
    tty = (struct tty *)devtab[dev].dvdata; /* and software params struct */

    if (baseport == COM1_BASE) {
	/* arm interrupts by installing int vec */
	set_intr_gate(COM1_IRQ+IRQ_TO_INT_N_SHIFT, &irq4inthand);
	/* commanding PIC to interrupt CPU for irq 4 (COM1_IRQ) */
	pic_enable_irq(COM1_IRQ);

    } else if (baseport == COM2_BASE) {
	/* arm interrupts by installing int vec */
	set_intr_gate(COM2_IRQ+IRQ_TO_INT_N_SHIFT, &irq3inthand);
	/* commanding PIC to interrupt CPU for irq 3 (COM2_IRQ) */
	pic_enable_irq(COM2_IRQ);
    } else {
	kprintf("ttyinit: Bad TTY device table entry, dev %d\n", dev);
	return;			/* give up */
    }
  //  tty->echoflag = 1;		/* default to echoing */

    init_queue( &(tty->tbuf), QMAX ); /* init tbuf Q */
    init_queue( &(tty->ebuf), QMAX ); /* init ebuf Q */
    init_queue( &(tty->rbuf), QMAX ); /* init rbuf Q */

    /* enable interrupts on receiver now, leave them on, but
       wait until output shows up to enable transmitter interrupts. */
    outpt(baseport+UART_IER, UART_IER_RDI); /* RDI = receiver data int */
}


/*====================================================================
*
*       tty-specific read routine for TTY devices
*
*/

int ttyread(int dev, char *buf, int nchar)
{
    //int baseport;
    struct tty *tty;
    int i = 0;
    int saved_eflags;		/* old cpu control/status reg, so can restore it */

    //baseport = devtab[dev].dvbaseport; /* hardware addr from devtab */
    tty = (struct tty *)devtab[dev].dvdata;   /* software data for line */

    while (i < nchar) { /* loop until we get user-specified # of chars */
	saved_eflags = get_eflags();
	cli();			/* disable ints in CPU */
				/* for queuecount, dequeue calls */
	if (queuecount( &(tty->rbuf) )) /* if there is something in rbuf */
	    buf[i++] = dequeue(&(tty->rbuf)); /* copy from rbuf Q to user buf */
	set_eflags(saved_eflags);     /* back to previous CPU int. status */
    }
    return nchar; 
}


/*====================================================================
*
*       tty-specific write routine for SAPC devices
*       
*/

int ttywrite(int dev, char *buf, int nchar)
{
    int baseport;
    struct tty *tty;
    int i = 0;
    int saved_eflags;

    baseport = devtab[dev].dvbaseport; /* hardware addr from devtab */
    tty = (struct tty *)devtab[dev].dvdata;   /* software data for line */

    saved_eflags = get_eflags();
    cli();			/* disable ints in CPU */
    /* load tx queue some to get started: this doesn't spin */
    while ((i < nchar) && (enqueue( &(tty->tbuf), buf[i])!=FULLQUE))
	i++;
    /* now tell transmitter to interrupt (or restart output) */
    outpt( baseport+UART_IER, UART_IER_RDI | UART_IER_THRI); /* enable both */
    /* read and write int's */
    set_eflags(saved_eflags);
    /* loop till all chars are gotten into queue, spinning as needed */
    while ( i < nchar ) {
		cli();			/* enqueue is critical code */
		if (enqueue( &(tty->tbuf), buf[i])!=FULLQUE) {
	    	i++;		/* success, advance one spot */
	//	    kickout(baseport);	/* make sure transmits enabled */
		    outpt(baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);
		} else {
			while (enqueue(&(tty->tbuf),buf[i]) == FULLQUE) {
				if (dev == TTY0) {
					sleep(TTY0_OUTPUT);
				} else {
					sleep(TTY1_OUTPUT);
				}
			}
			//i++;
			break;
			sti();
		}
		set_eflags(saved_eflags); /* restore CPU flags */
    }
    return nchar;
}


/*====================================================================
*
*       tty-specific control routine for TTY devices
*
*/

int ttycontrol(int dev, int fncode, int val)
{
    struct tty *this_tty = (struct tty *)(devtab[dev].dvdata);

    if (fncode == ECHOCONTROL)
	this_tty->echoflag = val;
    else return -1;
    return 0;
}

/*====================================================================
*
*       tty-specific interrupt routines for COM ports
*       Best to use a function for common code--
*/

void irq4inthandc()
{
    irqinthandc(TTY0);
}                              
  
void irq3inthandc()
{
    irqinthandc(TTY1);
}                              
                                  
/* Traditional UART treatment: check the devices' ready status
   on int, shutdown tx ints if nothing more to write.
   Note: never looks at IIR, is fairly generic */

void irqinthandc(int dev)
{
	int ch, lsr;
    char log[BUFLEN];
   
    struct tty *tty = (struct tty *)(devtab[dev].dvdata);
    int baseport = devtab[dev].dvbaseport; /* hardware i/o port */;
    pic_end_int();           /* notify PIC that its part is done */
    debug_log("~");

    lsr=inpt(baseport + UART_IIR);
    switch( lsr = lsr & UART_IIR_ID)
    { 
	// read
      case UART_IIR_RDI: 
      	ch = inpt(baseport+UART_RX); /*read char, ack the device */
       	enqueue( &tty->rbuf, ch ); /* save char in read Q (if fits in Q) */
       	//sprintf(log,"<%c", ch);
       	//debug_log(log);i
		debug_log("~a");
       
       	if (tty->echoflag){	/* if echoing wanted */
        	enqueue(&tty->ebuf,ch); /* echo char (if fits in Q) */
        	if (queuecount( &tty->ebuf )==1) { /* if first char...*/
				debug_log("~e");
				/* enable transmit interrupts also */      
         		 outpt( baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);
			}
       	}
       	break;
	// write
      case UART_IIR_THRI:
        if (queuecount( &tty->ebuf )) {
			/* if there is char in echo Q output it*/
        	outpt( baseport+UART_TX, dequeue( &tty->ebuf ) ); /* ack tx dev */
        } else if (queuecount( &tty->tbuf ))  {
	    	/* if there is char in tbuf Q output it */
            ch =dequeue(&tty->tbuf);
	       	//sprintf(log, ">%c", ch);
	       	//debug_log(log);
	       	outpt( baseport+UART_TX, ch ) ; /* ack tx dev */
			if (dev == TTY0) {
				wakeup(TTY0_OUTPUT);
			} else {
				wakeup(TTY1_OUTPUT);
			}
        } else {	
			/* all done transmitting */ 
			debug_log("~s");
            outpt( baseport+UART_IER, UART_IER_RDI); /* shut down tx ints */
        }
	    break;

        default:
	    	sprintf(log, "@%c",lsr +0x30); /* print out error: @1 to @7 except @2 or @4 */
	      	debug_log(log);
	   	} 
}

/*void debug_log(char*msg)
{
   strcpy(debug_record, msg);
   debug_record  +=strlen(msg);
   }
*/
