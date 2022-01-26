# CS444 hw2 makefile
# Usage: make U=test1   to build test1.lnx with tiny UNIX

# system directories needed for compilers, libraries, header files--
#
#     March 1, 2020
#     Ron Cheung
#
SAPC_TOOLS = /home/cheungr/serl/tutor-linux
PC_LIB = $(SAPC_TOOLS)/libc
PC_INC = $(SAPC_TOOLS)/include

PC_CC    = gcc
PC_CFLAGS  = -gdwarf-2 -gstrict-dwarf -march=i586 -m32 -fno-builtin -fno-stack-protector -nostdlib     -c -Wall -I$(PC_INC)
PC_AS    = as --32
PC_NM    = nm
PC_LD    = ld -m elf_i386


# File suffixes:
# .c	C source (often useful both for UNIX host and SAPC)
# .s 	assembly language source
# .o  relocatable machine code, initialized data, etc., for SA PC
# .lnx  executable image (bits as in memory), for SA PC (Linux a.out format)
# .syms text file of .lnx's symbols and their values (the "symbol table")
# Symbol file "syms"--for most recently built executable in directory

# PC executable--tell ld to start code at 0x100100, load special startup
# module, special PC C libraries--
U = uprog

# for CS444 hw2
# Object files for tiny unix OS
# You may need to add to these lists, and/or put in appropriate .o rules
#  in the later part of this file

IO_OFILES = io.o tty.o ioconf.o queue.o
KERNEL_OFILES = startup0.o startup.o tunix.o sysentry.o $(IO_OFILES)
USER_OFILES = crt0.o $(U).o ulib.o

#hw2 executable, for user program U--
# note that startup0.o needs to be first, so that "go 100100"
# executes it to start everything 
$(U).lnx: $(KERNEL_OFILES) $(USER_OFILES)
	$(PC_LD) -N -Ttext 100100 -o $(U).lnx \
	  $(KERNEL_OFILES) $(USER_OFILES) $(PC_LIB)/libc.a
	rm -f syms;$(PC_NM) -n $(U).lnx>$(U).syms;ln -s $(U).syms syms


$(U).o: $(U).c tunistd.h tty_public.h 
	$(PC_CC) $(PC_CFLAGS) -c -o $(U).o $(U).c

io.o: io.c ioconf.h
	$(PC_CC) $(PC_CFLAGS) -c -o io.o io.c

ioconf.o: ioconf.c ioconf.h
	$(PC_CC) $(PC_CFLAGS) -c -o ioconf.o ioconf.c

tty.o: tty.c tty.h tty_public.h
	$(PC_CC) $(PC_CFLAGS) -c -o tty.o tty.c

queue.o: queue/queue.c  queue/queue.h
	$(PC_CC) $(PC_CFLAGS) -c -o queue.o queue/queue.c

tunix.o: tunix.c
	$(PC_CC) $(PC_CFLAGS) -c -o tunix.o  tunix.c

# Note we can use gcc to assemble for us--
startup0.o: startup0.s
	$(PC_AS) -c -o startup0.o startup0.s

startup.o: startup.c
	$(PC_CC) $(PC_CFLAGS) -c -o startup.o startup.c

sysentry.o: sysentry.s
	$(PC_AS) -c -o sysentry.o sysentry.s

crt0.o: crt0.s
	$(PC_AS) -c -o crt0.o crt0.s

ulib.o: ulib.s
	$(PC_AS) -c -o ulib.o ulib.s

testio.o: testio.c io_public.h
	$(PC_CC) $(PC_CFLAGS) -c -o testio.o  testio.c

clean:
	rm -f *.o *.lnx core 

# "make spotless" to remove (hopefully) everything except sources
#  use this after grading is done
spotless:
	rm -f *.o *syms *.lnx




