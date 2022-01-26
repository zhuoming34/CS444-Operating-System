# file:  crt03.s
# name:  Zhuoming Huang
# description:   user startup module3 for main3
# date:  11/23/21

.globl ustart3, main3 , exit
.text

ustart3:        movl $0x3ffff0, %esp         #reinit the stack
                movl $0, %ebp                # and frame pointer
                call main3                   #call main3 in the uprog3.c
                pushl %eax                   #push the retval=exit_code on stack
                call exit                   # call sysycall exit
