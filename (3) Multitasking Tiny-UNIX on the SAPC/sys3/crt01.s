# file:  crt01.s
# name:  Zhuoming Huang
# description:   user startup module1 for main1
# date:  11/23/21

.globl ustart1, main1 , exit
.text

ustart1:        movl $0x3ffff0, %esp         #reinit the stack
                movl $0, %ebp                # and frame pointer
                call main1                   #call main1 in the uprog1.c
                pushl %eax                   #push the retval=exit_code on stack
                call exit                   # call sysycall exit
