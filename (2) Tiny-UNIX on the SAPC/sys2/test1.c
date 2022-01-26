// file: test1.c
// author: Zhuoming Huang
// perform same operations as in testio.c with system calls
// modified from testio.c

#include <stdio.h> /* for lib's device # defs, protos*/
#include "tunistd.h"
// phase out #include io_public.h as an user-callable API, replaced with syscalls

#define MILLION 1000000
#define DELAYLOOPCOUNT (400 * MILLION)
#define BUFLEN 80

void delay(void);

int main(void)
{
  char buf[BUFLEN];
  int got, i, lib_console_dev, ldev;
  // char echoctrl = ECHOCONTROL;

  // determine the SAPC's "console" device, the serial port for user i/o
  lib_console_dev = sys_get_console_dev(); // SAPC support lib fn
  switch (lib_console_dev) {
  	case COM1: 
		ldev = TTY0; // convert to our dev #'s
		break;
	case COM2: 
		ldev = TTY1;
		break;
	default:
		printf("unknown console device\n");
		return 1;
  }
  kprintf("running with device TTY%d\n", ldev);
  // new have a usable device to talk to with i/o package

  // remove ioinit(); // initialize devices
  
  kprintf("\ntrying simple write (4 chars) ...\n");
  got = write(ldev, "hi!\n", 4);
  kprintf("write of 4 returned %d\n", got);
  delay(); // time for output to finish, once write-behind is working


  kprintf("\ntrying longer write (9 chars) ...\n");
  got = write(ldev, "abcdefghi", 9);
  kprintf("write of 9 returned %d\n", got);
  delay(); // time for output to finish, once write-behind is working


  for (i=0; i<BUFLEN; i++) {
  	buf[i] = 'A' + i/2;
  }
  kprintf("\ntrying write of %d-char string...\n", BUFLEN);
  got = write(ldev, buf, BUFLEN);
  kprintf("\nwrite returned %d\n", got);
  delay();


//  scanf("turn of echo? [y/n]\n", echoctrl);
//  if (echoctrl == 'y')
//	  control(ldev, ECHOCONTROL, 1);
//  else if (echoctrl == 'n')
//          control(ldev, ECHOCONTROL, 0);
//  else
//          kprintf("not available\n");
  // system call for echo control is not available, so turn it off
  kprintf("echo control syscall is not available\n");
  kprintf("echo is off\n");
//  control(ldev, ECHOCONTROL, 0);


  kprintf("\nType 10 chars input to test typeahead while looping for delay...\n");
  delay();
  got = read(ldev, buf, 10);    /* should wait for all 10 chars, once fixed */
  kprintf("\nGot %d chars into buf. Trying write of buf...\n", got);
  write(ldev, buf, got);


  kprintf("\nTrying another 10 chars read right away...\n");
  got = read(ldev, buf, 10);    /* should wait for input, once fixed */
  kprintf("\nGot %d chars on second read\n",got);
  if (got == 0)
  	kprintf("nothing in buffer\n");   /* expected result until fixed */
  else
  	write(ldev, buf, got);    /* should write 10 chars once fixed */


  kprintf("\nType 20 chars input, note lack of echoes...\n");
  delay();
  got = read(ldev, buf, 20);
  kprintf("\nTrying write of buf...\n");
  write(ldev, buf, got);
  kprintf("\nAsked for 20 characters; got %d\n", got);

  kprintf("\nend of test\n");

  return 0;
}


/* the faster the machine you are on, the longer this loop must be */
void delay()
{
  int i;
  kprintf("<doing delay>\n");
  for (i = 0; i < DELAYLOOPCOUNT; i++)
	 ; 
}


