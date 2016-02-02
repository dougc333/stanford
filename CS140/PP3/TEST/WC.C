//word count test program.


#include "syscall.h"

//#include "stdio.h"
void printd(int n);
void prints(char *s);
int strlen(char *s);

#define IN 1
#define OUT 0

int main(int argc, char *argv[])
{
  OpenFileId source;
  int length=0;
  int i;
  int newlines=0;
  int words=0;
  int linesOnly=0;
  int state;
  char buffer[1];
  char *file;

  if (argc==1 || argc > 3) {
    prints("Usage: wc [-l] [file]\n");
    return -1;
  }

  if (argc==2) {
    if((source=Open(argv[1]))==-1) {
      prints("wc: Error opening file\n");
      return -1;
    }
    file=argv[1];
  }

  if (argc==3) {
    if (argv[1][0]=='-' && argv[1][1]=='l') {
      linesOnly=1;
      if((source=Open(argv[2]))==-1) {
	prints("wc: Error opening file\n");
	return -1;
      }
      file=argv[2];
    } else {
      prints("Usage: wc [-l] [file]\n \n");
      return -1;
    }
  }

  length=Length(source);
  state=OUT;
  i=0;
  while(i<length) {
    Read(buffer, 1, source);
    i++;
    if (buffer[0]=='\n') {
      newlines++;
    }
    if (buffer[0]==' ' || buffer[0]=='\n' || buffer[0]=='\t')
      state=OUT;
    else if (state==OUT) {
      state=IN;
      words++;
    }
  }
  
  Close(source);
     
  if (linesOnly){
    printd(newlines);
    prints (" ");
  }
  else {
    printd(newlines);
    prints (" ");
    printd(words);
    prints (" ");
    printd(length);
    prints (" ");
  }

  prints (file);
  prints ("\n");
}

void prints(char *s) {
  Write(s, strlen(s), 1);
//prints("\nContent of Doug\n");
}


int strlen(char *s) {
  int i;

  i = 0;
  while (s[i] != '\0')
    ++i;
  return i;
}

void printd(int n) {
  if (n < 0) {
    Write("-", 1, 1);
    n=-n;
  }
  if (n / 10) {
    printd(n / 10);
  }

  switch(n%10) {
    case 0: Write("0", 1, 1); break;
    case 1: Write("1", 1, 1); break;
    case 2: Write("2", 1, 1); break;
    case 3: Write("3", 1, 1); break;
    case 4: Write("4", 1, 1); break;
    case 5: Write("5", 1, 1); break;
    case 6: Write("6", 1, 1); break;
    case 7: Write("7", 1, 1); break;
    case 8: Write("8", 1, 1); break;
    case 9: Write("9", 1, 1); break;
  }
}
