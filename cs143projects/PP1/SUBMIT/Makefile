## 
## Simple makefile for CS143 programming projects
##


.PHONY: clean strip

# Set the default target. When you make with no arguments,
# this will be the target built.
TARGET = dcc
PREPROCESSOR = dpp
default: $(TARGET)

# "make pure" will build a dcc.purify version of the executable
# which will execute much more slowly but have Purify's runtime
# memory protection checking on. Might be useful for debugging
pure: $(TARGET).purify

# Set up the list of source and object files
SRCS = utility.cc main.cc hash.c

# OBJS can deal with either .cc or .c files listed in SRCS
OBJS = lex.yy.o $(patsubst %.cc, %.o, $(filter %.cc,$(SRCS))) $(patsubst %.c, %.o, $(filter %.c, $(SRCS)))

JUNK =  *.o lex.yy.c dpp.yy.c y.tab.c y.tab.h *.core core $(TARGET).purify purify.log 

# Define the tools we are going to use
CC= g++
LD = g++
LEX = flex
YACC = bison

# Set up the necessary flags for the tools

# We want debugging and most warnings, but lex/yacc generate some
# static symbols we don't use, so turn off unused warnings to avoid clutter
# Also STL has some signed/unsigned comparisons we want to suppress
CFLAGS = -g -Wall -Wno-unused -Wno-sign-compare

# The -d flag tells lex to set up for debugging. Can turn on/off by
# setting value of global yy_flex_debug inside the scanner itself
LEXFLAGS = -d

# The -d flag tells yacc to generate header with token types
# The -v flag writes out a verbose description of the states and conflicts
# The -t flag turns on debugging capability
# The -y flag means imitate yacc's output file naming conventions
YACCFLAGS = -dvty

# Link with standard c library, math library, and lex library
LIBS = -lc -lm -ll

# Rules for various parts of the target

.yy.o: $*.yy.c
	$(CC) $(CFLAGS) -c -o $@ $*.cc

lex.yy.c: scanner.l 
	$(LEX) $(LEXFLAGS) scanner.l

.cc.o: $*.cc
	$(CC) $(CFLAGS) -c -o $@ $*.cc

# rules to build compiler (dcc)

$(TARGET) : $(PREPROCESSOR) $(OBJS)
	$(LD) -o $@ $(OBJS) $(LIBS)

$(TARGET).purify : $(OBJS)
	purify -log-file=purify.log -cache-dir=/tmp/$(USER) -leaks-at-exit=no $(LD) -o $@ $(OBJS) $(LIBS)


# rules to build preprocessor (dpp)
PREP_OBJS = dpp.yy.o dppmain.o utility.o hash.o

$(PREPROCESSOR) : $(PREP_OBJS)
	$(LD) -o $@ $(PREP_OBJS) $(LIBS)

dpp.yy.c : dpp.l
	$(LEX) -odpp.yy.c dpp.l


# This target is to build small for testing (no debugging info), removes
# all intermediate products, too
strip : $(TARGET) $(PREPROCESSOR)
	strip $(TARGET) $(PREPROCESSOR)
	rm -rf $(JUNK)


# make depend will set up the header file dependencies for the 
# assignment.  You should make depend whenever you add a new header
# file to the project or move the project between machines
#
depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

clean:
	rm -f $(JUNK) y.output $(TARGET) $(PREPROCESSOR)

