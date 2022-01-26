/*********************************************************************
*
*       file:           tty.c
*       author:         betty o'neil
*                       Ray Zhang
*       date:           02/19/97
*
*       tty driver--device-specific routines for ttys 
*
*   eoneil: cleaned up a few things, made it work using the traditional
*   tx interrupt method on TTY1 and the 16450-specific way using IIR
*   on TTY0.
*/

#include <stdio.h> /* for kprintf, sprintf */
#include <serial.h>
#include <cpu.h>
#include <pic.h>
#include "tsystm.h"
#include "ioconf.h"
#include "tty_public.h"
#include "tty.h"
#include "proc.h"
#include "sched.h"

/* define maximum size of queue */
#define QMAX 6
/* for debug string buffer */
#define BUFLEN 20

/* tell C about the assembler shell routines */
extern void irq3inthand(void), irq4inthand(void);

/* C part of interrupt handlers--specific names called by the assembler code */
extern void irq3inthandc(void), irq4inthandc(void); 

/* the common code for the two interrupt handlers */ 
static void irqinthandc(int dev); 

struct tty ttytab[NTTYS]; /* software params/data for each SLU dev */


/*====================================================================
*
*       tty specific initialization routine for COM devices
*
*/

void ttyinit(int dev)
{
  int baseport;
  struct tty *tty;		/* ptr to tty software params/data block */

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
  tty->echoflag = 1;		/* default to echoing */

  init_queue( &(tty->tbuf), QMAX ); /* init tbuf Q */
  init_queue( &(tty->ebuf), QMAX ); /* init ebuf Q */
  init_queue( &(tty->rbuf), QMAX ); /* init rbuf Q */

  /* enable interrupts on receiver now, later for transmitter */
  outpt(baseport+UART_IER, UART_IER_RDI);
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
  int saved_eflags;        /* old cpu control/status reg, so can restore it */

  //baseport = devtab[dev].dvbaseport; /* hardware addr from devtab */
  tty = (struct tty *)devtab[dev].dvdata;   /* software data for line */

  while (i < nchar) { /* loop until we get user-specified # of chars */
    saved_eflags = get_eflags();
    cli();			/* disable ints in CPU */

    if (queuecount( &(tty->rbuf) )) /* if there is something in rbuf */
      buf[i++] = dequeue(&(tty->rbuf));    /* copy from ibuf to user buf */

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
  {
    debug_log("b");
    i++;
  }

  /* now tell transmitter to interrupt (or restart output) */
  outpt( baseport+UART_IER, UART_IER_RDI | UART_IER_THRI); /* enable both */
                                                 /* read and write int's */
//  set_eflags(saved_eflags);

  /* loop till all chars are gotten into queue, spinning as needed */
  while ( i < nchar ) {
    saved_eflags = get_eflags();
    cli();			  /* enqueue is critical code */
    debug_log("b");
    while(enqueue(&(tty->tbuf), buf[i]) == FULLQUE){
	/* give up CPU until the queue becomes available */
	sleep(TTY1_OUTPUT); 
    }
//    set_eflags(saved_eflags);     /* restore CPU flags */
    i ++;   /* success, advance one spot */
    outpt( baseport+UART_IER, UART_IER_RDI | UART_IER_THRI); /* enable both */
  }
    set_eflags(saved_eflags);
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

void irq4inthandc()
{
  irqinthandc(TTY0);
}                              
  
void irq3inthandc()
{
  irqinthandc(TTY1);
}                              


/*====================================================================
*
*       tty-specific interrupt routine for COM ports
*
*/

/* For hw1 solutions:
   --with USE_SECONDARY_METHOD defined, use secondary TX ack method, 
         reading IIR to do ack.
   --by default, use traditional UART method, shutting down ints
        when out of chars to ack TX with
   (student solutions could use either method)
*/

/* uses secondary TX ack method by reading the IIR register
   that is special to this UART, the 16450/16550 */
/* NOTE: by default, USE_SECONDARY_METHOD is not defined. */

void irqinthandc(int dev)
{
  int ch, iir;
  struct tty *tty = (struct tty *)(devtab[dev].dvdata);
  int baseport = devtab[dev].dvbaseport; /* hardware i/o addr */;

  pic_end_int();                /* notify PIC that its part is done */
  /* read IIR: this acks tx ints (but not rx ints) for this UART */
  /* The IIR reports current rx int, or if none, tx int */
  iir = inpt(baseport+UART_IIR);

  switch (iir&UART_IIR_ID) {	/* mask the 2-bit ID field */

  case UART_IIR_RDI:		/* it's a receiver int */
    ch = inpt(baseport+UART_RX);      /* read char, ack the device */
    enqueue( &tty->rbuf, ch );       /* save char in Q */
    if (tty->echoflag) {               /* if echoing wanted */
      enqueue(&tty->ebuf,ch);	      /* echo char */
      if (queuecount( &tty->ebuf )==1)  /* if first char...*/
	outpt( baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);
    }
    break;

  case UART_IIR_THRI:  /* it's a transmit int */
    debug_log("~");     /* mark event in debug log in memory */
    if (queuecount( &tty->ebuf )){  /* if there is char in echo Q */
      outpt( baseport+UART_TX, dequeue( &tty->ebuf ) );
      debug_log("e");           /* mark echo (but not ordinary output) */
    } else if (queuecount( &tty->tbuf )){ /* if there is char in ouput Q */
      outpt( baseport+UART_TX, dequeue( &tty->tbuf ) );
    /* signal waiting process that the queue becomes available */
      wakeup(TTY1_OUTPUT);
      debug_log("x");
    } else{
       outpt( baseport+UART_IER, UART_IER_RDI); /* shut down tx ints */
       debug_log("s");           /* mark shutdown */
      } 
/* else can return, tx dev already ack'd by read of IIR */ 
    break;

  default: kprintf("irqinthandc: funny IIR %x\n", iir);
  }
}                                 
                                  



