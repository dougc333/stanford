/* File: dppmain.cc
 * ----------------
 * This file defines the main() routine for the preprocessor, 
 * the filtering tool which runs before the compiler.
 */
 
#include "scanner.h"


/*
 * Function: main()
 * ----------------
 * Entry point to the preprocessor.
 * As given below, this "filter" just copies stdin to stdout unchanged.
 * It will be your job to actually do some filtering, either by hand-coded
 * transformation of the input or by setting up a lex scanner in the dpp.l
 * file and changing the main below to invoke it via yylex. When finished,
 * the preprocessor should echo stdin to stdout making the transformations
 * to strip comments and handle preprocessor directives. Since the comments
 * are stripped out, we can store each character in a buffer till we see
 * a word, then we search the hashtable once we detect a word. 
 * 
 */
int main(int argc, char *argv[])
{
  InitPreProcessor();
  while (yylex()!=0) { 
    char ch = getc(stdin);
    putc(ch,stdout);
  }
  return 0;
}


