##
## Makefile for CS107 hw0
##
## If you just say "make" it makes the stacktest.
##
##

CPPFLAGS = -g -Wall

CXX = /usr/newsw/bin/g++

LDFLAGS =

SRCS = main.cc stack.cc

OBJS = $(SRCS:.cc=.o)

PROGS = stacktest

default : stacktest

pure : stacktest.purify

stacktest :  $(OBJS)
	$(CXX) -o $@ $(OBJS)   $(LDFLAGS)

stacktest.purify :  $(OBJS)
	purify $(CXX) -o $@ $(OBJS)   $(LDFLAGS)

# The dependencies below make use of make's default rules,
# under which a .o automatically depends on its .c and
# the action taken uses the $(CC) and $(CFLAGS) variables.
# These lines describe a few extra dependencies involved.

depend:: Makefile.dependencies $(SRCS)

Makefile.dependencies:: $(SRCS)
	$(CXX) $(CPPFLAGS) -MM $(SRCS) > Makefile.dependencies

-include Makefile.dependencies

clean : 
	/bin/rm -f *.o *.purify a.out core $(PROGS) Makefile.dependencies

TAGS : $(SRCS)
	etags -t $(SRCS)

