# file:	 crt03.s
# name:	 joseph tam phui hui
# description:	 user startup module	
# date:	 3-3-97

.globl ustart3, main3, exit
.text
			
ustart3:	call main3                  # call main in the uprog3.c
                pushl %eax                   # push the retval=exit_code on stack
                call exit                   # call sysycall exit
               

