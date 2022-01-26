# file:	 crt01.s
# name:	 joseph tam phui hui
# description:	 user startup module	
# date:	 3-3-97

.globl ustart1, main1, exit
.text
			
ustart1:	call main1                  # call main in the uprog1.c
                pushl %eax                   # push the retval=exit_code on stack
                call exit                   # call sysycall exit
               

