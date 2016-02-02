//copy test program


#include "syscall.h"
#define BUFSIZE 10
void printd(int n);
void prints(char *s);
int strlen(char *s);
int main(int argc, char *argv[])
{
  OpenFileId source, dest;
  char buf[BUFSIZE];
  int   n,i;

  if (argc!=2) {
    prints("Usage: cat SOURCE\n");
    return -1;
  }

  if((source=Open(argv[1]))==-1) {
    prints("cat: Error opening source file\n");
    return -1;
  }
  i=0;
  while ((i<20)&&((n=Read(buf, BUFSIZE, source)) > 0 )) {
    prints("\n");
    printd(n);
    prints("\n");
    Write(buf, n, 1);
    i++;
  }

  Close(source);
  return 0;
}

void prints(char *s) {
  Write(s, strlen(s), 1);
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
