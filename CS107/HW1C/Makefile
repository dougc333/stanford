##
## Makefile for CS107 hw1c
##
## Makes rsg and rsg.purify
##

## Set the compiler to be a known ANSI compliant one
CC = gcc

## adding the '-DNDEBUG' defines the CT symbol NDEBUG which
## suppresses all the asserts and what not. You'd do that for
## a production build, but it is a good idea to leave it in
## for the purposes of this class.
DFLAG = ## -DNDEBUG

## the -I allows you to include any local header files for our
## class libraries.  
CFLAGS = -g -Wall -Wpointer-arith $(DFLAG) -I/usr/class/cs107/include
LDFLAGS = -g
PUFLAGS = -cache-dir=/tmp/$(USER)

HDRS = darray.h hashtable.h scanner.h
SRCS = darray.c hashtable.c scanner.c rsg.c
OBJS = $(SRCS:.c=.o)
TARGET = rsg

default : $(TARGET)

pure : rsg.purify

rsg : $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o rsg

rsg.purify : $(OBJS)
	purify $(PUFLAGS) $(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o rsg.purify


# The dependencies below make use of make's default rules,
# under which a .o automatically depends on its .c and
# the action taken uses the $(CC) and $(CFLAGS) variables.
# These lines describe a few extra dependencies involved.

darray.o : darray.h
hashtable.o : hashtable.h
rsg.o : darray.h hashtable.h scanner.h

clean : 
	@echo "Removing all object files..."
	rm -f core *.o *.purify $(TARGET) purify.log