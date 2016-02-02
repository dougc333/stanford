##
## Makefile for CS154 Project
##

# Set the compiler to be a known ANSI compliant one
# Actually go back a version and use 2.7.2 instead of 2.8
# since it plays better with Purify
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
CFLAGS = -g -Wall $(DFLAG) 
LDFLAGS = -g 

TARGET = parser

default : $(TARGET) 

HDRS =  parser.h
SRCS =  parser.c test.c
OBJS = $(SRCS:.c=.o)
PROGS = parser
pure : parser.purify

parser : $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o parser

parser.purify :  $(OBJS)
	purify -log-file=purify.log -cache-dir=/tmp/$(USER) $(CC) $(CFLAGS) -o $@ $(OBJS)   $(LDFLAGS)

# The dependencies below make use of make's default rules,
# under which a .o automatically depends on its .c and
# the action taken uses the $(CC) and $(CFLAGS) variables.
# These lines describe a few extra dependencies involved.

parser.o : parser.h
test.o : parser.h

clean :
	@echo "Removing object files..."
	/bin/rm -f *.o a.out core $(PROGS) *.purify purify.log

TAGS : $(SRCS) $(HDRS)
	etags -t $(SRCS) $(HDRS)
