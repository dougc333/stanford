#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main (int argc, char **argv) {
  FILE *infile, *outfile;
  char outfilename[512], outbuf[4];
  int c;

  outbuf[0] = '#';

  assert (argc == 2);

  strncpy(outfilename, argv[1], 506);
  strcat(outfilename, ".asdf");
  // create a new file with same name as old but extension .asdf

  infile = fopen(argv[1], "r");

  outfile = fopen(outfilename, "r");
  assert (!outfile);
  // We don't want to destroy any of your precious files!

  outfile = fopen(outfilename, "w");

  assert (infile);
  assert (outfile);
  
  while ( (c = getc(infile)) != EOF) {
    outbuf[1] = c / 100 + '0';
    outbuf[2] = (c / 10) % 10 + '0';
    outbuf[3] = c % 10 + '0';
    
    putc(outbuf[0], outfile);
    putc(outbuf[1], outfile);
    putc(outbuf[2], outfile);
    putc(outbuf[3], outfile);
  }

  fclose(infile);
  fclose(outfile);
  return 0;
}
