# crt0.s: asm ustart function
# modified from startup0.s
.text
.global _ustart, main, exit
_ustart: movl $0x3ffff0, %esp	# set user program stack
	 movl $0, %ebp		# make debugger backtrace terminate here
	 call main		# main in c
	 pushl %eax		# exitcode
	 call exit		# syscall exit
