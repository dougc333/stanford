#include "parser.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
  FILE *infile;
  char *expstr;
  RegExpressionT *ex;

  if (argc != 2) {
    printf("Improper command line format\n");
    return 0;
  }

  infile = fopen(argv[1], "r");
  if (!infile) {
    printf("File opening failed\n");
    return 0;
  }

  while (1) {
    expstr = ReadFromFile(infile);
    if (!expstr) break;
    ex = ParseRegExp(expstr);
    TestRegExp(ex);
    FreeRegularExpression(ex);
    free(expstr);
  }

  fclose(infile);

  return 0;
}

