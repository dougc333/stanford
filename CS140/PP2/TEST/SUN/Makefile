# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as
GCCDIR = /usr/class/cs140/tools/mips-dec-ultrix/bin/
LDFLAGS = -T ../script -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)
BINDIR = ../../bin/SUN


include ../Makefile.ALL


