# sysentry.s
# trap handler's asm envelope routine syscall for the trap resulting
# from a system call, need to push eax, ebx, ecx, edx on the stack
# where they can be accessed from C.
# call syscallc, then pops, iret.
# modified from irq4.s and lec9 pg11

.text
.globl _syscall, syscallc

_syscall: pushl %edx # push the values of eax to edx to stack
	  pushl %ecx
	  pushl %ebx
	  pushl %eax
	  call syscallc # call c trap routine in tunix.c
	  popl %eax 	# or replace it with add $4, %esp to 
			# skip over the eax from the stack
			# pop the values of ebx to edx from stack
	  popl %ebx
	  popl %ecx
	  popl %edx
	  iret


