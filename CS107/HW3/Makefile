##
## Makefile for all the concurrency examples
##

## Set the compiler to be a known ANSI compliant one
CC = gcc

## adding the '-DNDEBUG' defines the CT symbol NDEBUG which
## suppresses all the asserts and what not. You'd do that for
## a production build, but it is a good idea to leave it in
## for the purposes of this class.
DFLAG = ## -DNDEBUG

## the -D_REENTRANT is for the library routines
CFLAGS = -D_REENTRANT -g -Wall $(DFLAG) -I/usr/class/cs107/include
LDFLAGS = -g -L/usr/class/cs107/lib -lthread_107 -lnetwork -lthread -lposix4 -lnsl -lsocket
PFLAGS = -max_threads=400 -cache-dir=/tmp -log-file=purify.log

SRCS = sweethand.c scanner.c
HDRS = scanner.h
OBJS = $(SRCS:.c=.o)
TARGET = sweethand

default : $(TARGET)

sweethand :  $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $@


# The dependencies below make use of make's default rules,
# under which a .o automatically depends on its .c and
# the action taken uses the $(CC) and $(CFLAGS) variables.
# These lines describe a few extra dependencies involved

scanner.o : scanner.h
sweethand.o : scanner.h

clean : 
	@echo "Removing all object files..."
	/bin/rm -f *.o a.out core $(TARGET) factory
	\rm -fi *~

TAGS : $(SRCS) $(HDRS)
	etags -t $(SRCS) $(HDRS)
