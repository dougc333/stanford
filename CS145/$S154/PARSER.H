#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdio.h>

/* Regular Expression Parser */

/*
Introduction:

The complete grammar of this regular expression parser (E is the root):
E -> A|E or A
A -> BA or B
B -> C* or C+ or C? or C
C -> [D] or S or (E)
D -> SD or RD or S or R
R -> F-F (see note below)
S -> F or {W} or . or &
F -> any non-special character or \X or #NNN
X -> any character including special characters (see below)
W -> TW or T
T -> s or n or t or f or r (see below)

note: in R, the first symbol must be less than the second symbol (as C
compares characters).  Whitespace is represented by curly bracketed
sets of characters where s = space, n = newline, t = tab, f = formfeed,
and r = return.  All other whitespace is ignored.
The special characters are the characters of our regular expression 
metalanguage and a few reserved characters in case  you want to extend
the functionality.  The complete list of special characters follows:
&+*|?[]().-{}#/\ and reserved characters ^!$_:;,
The characters @'"<> are not reserved, since they're commonly used in 
web documents.
The grammar is explained in detail below.

Add functionality if you desire, though any regular expression should
be able to be contstructed using the following rules.  If you change the
parser, the parser should still handle any regular expression formed in
the original grammar, and you should include the changes made in your
comments.

Define: variables E, A, B to be regular expressions, variable a to stand
for any symbol represented by a one byte character, and r stand for a
valid range (a-z, A-Z, 0-9, or, in general, c1-c2 where c1, c2 are
characters and c1 < c2 (in the 'C' sense) -- note: you must use escape 
character '\' for special characters.)

A regular expression is:
Basis:  E = a - an atomic symbol
        E = \a - the escape character (used to literally mean the
         character 'a', as opposed to the meaning of a in the metalanguage
         of our regular expression.  Note that if you're defining strings
	 in C to stand for a regular expression and you'd like to use an
	 escape character in your string, you have to escape the escape
	 character '\' in C to get the correct results.
        E = [(r|a)+] - here I'm using our regular expression language 
	 to describe the contents of a character set.  Within the
character
         set you may have any number of ranges (c1-c2, c1<c2) and any
         number of single atomic symbols (a) including escaped or ASCII 
	 specified characters.  The character set then represents a 
	 transition on any of those characters.  For instance, the set
	 [0-9abc] represents the character set
{0,1,2,3,4,5,6,7,8,9,a,b,c}.
	E = {(s|n|t|f|r)+} - represents whitespace
        E = . - represents the complete character set, or any character.
        E = & - represents the empty string.
	E = #NNN where N is a digit 0-9 and the number represented by NNN
	 is the valid decimal representation of a character in the ASCII
	 character set.  This allows you to specify non-printing
characters
	 in your regular expression such as '\0' (#000).  

Expressions (all of the following are supported):
        E = AB - implicit concatenation of two expressions is an
         expression.
        E = A|B - the union of two expressions is an expression - this
         represents that either A or B is true (or both).
        E = A* - the Kleene closure of an expression represents the
         reflexive, transitive closure of an expression, or the arbitrary
         concatenation of one or more of an expression.
        E = A+ - the positive closure of an expression is equivalent to
         the expression AA* (one or more repetitions of A).
        E = A? - represents one or zero instances of the expression A.
        E = (A) - allows determination of order of operation.

There are many possible extensions to this schema that you could implement
on your own.  These include looking ahead or back for certain expressions
to be true, repeating an expression a specific number of times, adding
sophisticated conditional or pattern matching functionality such as 
enforcing equal count of repetitions in two different places in an 
expression or the equal count of two characters throughout the string.  
Some could be quite simple like allowing octal or hex character
representation.

Some extensions could enhance the power of your pattern matching engine so
that it no longer strictly represents a regular language, though often 
times you will find yourself desiring extra power while searching for 
information within your data.  All of the code is available to you for you 
to play with, so any extensions on this (or if you want to rewrite your 
own parser) are perfectly fine.  You *will* have to expand the grammar to
accommodate for certain extensions.  Extra special characters have been
reserved for this purpose, though any change will require you to alter the
parser on a fairly fundamental level.  I've tried to make this as easy as
possible, though I cannot predict all of the different ways in which you 
might want to expand the program.  Be warned, though, that any extension
that exceeds the power of a DFA will necessarily change the method of 
translation into a workable machine, and your pattern matching engine will
be expected to accept any regular expression accepted by the original 
grammar and return the same output.

Cheers!
Your loving TA,
Nathan Folkert
*/

/* Type definitions and defined constants */

/* Most of these you won't have to touch unless you want to alter the 
parser or grammar of the regular expression itself */

typedef enum {
	EMPTY, /* empty string */
	ATOM, /* atomic element - a single character */
	ANYCHAR, /* useful union of all possible atomic elements */
	CHARSET, /* useful union of specific ranges of atomic elements */
	CONCAT, /* concatenation of two expressions */
	UNION, /* union of two expressions */
	KLEENE, /* Kleene closure of one expression */
	POSCLOSE, /* positive closure of one expression */
	OPTION /* one or zero repetitions of one expression */
} ExpType;

typedef struct CharSetT {
	int bytearr[32/sizeof(int)];
} CharSetT;

/* The regular expression definition contains extra information in
the form of pointers.  A union type could be devised, but it seemed
that this would only complicate your lives.  We can waste a little 
extra memory for your comfort.  Each expression represents a node
in the expression tree. */

typedef struct RegExpression {
	ExpType type;
	int value; /* Represents character value */
	CharSetT *charset;
	struct RegExpression *term; /* Represents first term in binary 
		operations or only term in unary operations */
	struct RegExpression *next; /* Represents second term in binary
		operations */
} RegExpressionT;

/* Utility Functions */

/* These functions are here for you to get the most out of your
parser.  Extend or use them in any way you see fit */

/* ParseRegExp(rxstr) */
/* This takes a string and produces a regular expression from it.
Allocates memory for the regular expression, and returns NULL on
an error */

RegExpressionT *ParseRegExp(char *rxstr);

/* ReadFromFile(infile) */
/* This can be used to read a regular expression from a file into
a string that you can then parse.  You can specify the infile as 
stdin if you want to produce your own regular expressions.  The 
format should be regular expressions of the format specified at
the top of this file.  This will read until it reaches a blank line
(two adjacent new lines).  Note that all internal whitespace will
be ignored by the parser.  For instance, on the four lines:
a|b*

a|
b*
Calling this function once would produce "a|b*\n", while calling 
it a second time on the same file would produce "a|\nb*\n"
Allocates memory for your string.
*/

char *ReadFromFile(FILE *infile);

/* InCharSet(c, h) */
/* Determines if a character is in a given character set */

int InCharSet(int c, CharSetT *h);

/* CreateCharSet() */
/* Allocates a new character set accepting no characters */

CharSetT *CreateCharSet(void);

/* Add/RemoveCharacter(c, h) */
/* These allow addition or removal of characters from a
character set. */

void AddCharacter(int c, CharSetT *h);
void RemoveCharacter(int c, CharSetT *h);
void AddCharacterRange(int a, int b, CharSetT *cs);
void RemoveCharacterRange(int a, int b, CharSetT *cs);
void InvertCharSet(CharSetT *h);

/* FreeRegularExpression(regexp) */
/* Frees all memory associated with a regular expression */
void FreeRegularExpression(RegExpressionT *regexp);

void TestRegExp(RegExpressionT *regexp);
void PrintCharSet(CharSetT *cs);

#endif
