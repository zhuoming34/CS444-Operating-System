# file:	 crt02.s
# name:	 joseph tam phui hui
# description:	 user startup module	
# date:	 3-3-97

.globl ustart2, main2, exit
.text
			
ustart2:	call main2                  # call main in the uprog2.c
                pushl %eax                   # push the retval=exit_code on stack
                call exit                   # call sysycall exit
               

