//copy test program


#include "syscall.h"
#define BUFSIZE 256

void prints(char *s);
int strlen(char *s);
int main(int argc, char *argv[])
{
  OpenFileId source, dest;
  char buf[BUFSIZE];
  int   n;

  if (argc!=3) {
    prints("Usage: cp SOURCE DESTINATION\n");
    return -1;
  }

  if((source=Open(argv[1]))==-1) {
    prints("cp: Error opening source file\n");
    return -1;
  }
  Create(argv[2]);
  if((dest=Open(argv[2]))==-1) {
    prints("cp: Error creating destination file\n");
    return -1;
  }
  
  
  while ((n=Read(buf, BUFSIZE, source)) > 0 ) {
    Write(buf, n, dest);
  }

  Close(source);
  Close(dest);
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

