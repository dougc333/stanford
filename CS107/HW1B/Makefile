##
## Makefile for CS107 hw1b
##
## There are 2 separate targets in this makefile.
## "adttest" compiles the provided adttest.c against your darray & hashtable
## "search" compiles all the home page search modules (including scanner)
##
## If you just say "make" it makes both. If you want to build just one
## you can do by indicating "make search" or "make adttest"
##
## There are also purify targets. You can make both with "make pure" 
## or just build one or the other with "make adttest.purify" or
## "make search.purify"

## Set the compiler to be a known ANSI compliant one
CC = gcc

## adding the '-DNDEBUG' defines the CT symbol NDEBUG which
## suppresses all the asserts and what not. You'd do that for
## a production build, but it is a good idea to leave it in
## for the purposes of this class.
DFLAG = ## -DNDEBUG

## the -I allows you to include any local header files for our
## class libraries.  The define of __USE_FIXED_PROTOTYPES__ is
## essential for convincing the compiler to use the ANSI
## header files (i.e. with proper prototypes) that ship with gcc
CFLAGS = -g -Wall -Wpointer-arith $(DFLAG) -I/usr/class/cs107/include
LDFLAGS = -g
PUFLAGS = -log-file=purify.log -cache-dir=/tmp/$(USER)

HDRS = darray.h hashtable.h scanner.h
ADT_SRCS = darray.c hashtable.c
ADTTEST_OBJS = $(ADT_SRCS:.c=.o) adttest.o
SEARCH_OBJS = $(ADT_SRCS:.c=.o) scanner.o search.o
PROGS = adttest search
PURIFY_PROGS = adttest.purify search.purify

default : $(PROGS)

pure : $(PURIFY_PROGS)

adttest : $(ADTTEST_OBJS)
	$(CC) $(ADTTEST_OBJS) $(LDFLAGS) -o adttest

adttest.purify : $(ADTTEST_OBJS)
	purify $(PUFLAGS) $(CC) $(CFLAGS) $(ADTTEST_OBJS) $(LDFLAGS) -o adttest.purify

search : $(SEARCH_OBJS)
	$(CC) $(SEARCH_OBJS) $(LDFLAGS) -o search

search.purify : $(SEARCH_OBJS)
	purify $(PUFLAGS) $(CC) $(CFLAGS) $(SEARCH_OBJS) $(LDFLAGS) -o search.purify


# The dependencies below make use of make's default rules,
# under which a .o automatically depends on its .c and
# the action taken uses the $(CC) and $(CFLAGS) variables.
# These lines describe a few extra dependencies involved.

darray.o : darray.h
hashtable.o : hashtable.h
adttest.o : darray.h hashtable.h 
search.o : darray.h hashtable.h scanner.h

clean : 
	@echo "Removing all object files..."
	rm -f core *.o *.purify $(PROGS) purify.log