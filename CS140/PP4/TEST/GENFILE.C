//  genfile
#define BlockSize 128
#include "syscall.h"
char progname[]="This is the GenFile program";
void prints(char *s);
int strlen(char *s);
void printd(int n);
int atoi(char s[]);
void main(int argc, char *argv[])
{
  char G[BlockSize];
  int i;
  int fileId;
  int fileSize;
  int n;
  
  if (argc !=3) {
    prints("Usage: genfile <filename> <size in blocks of 128 bytes>\n");
    return;
  }

  fileSize = atoi(argv[2]);
  
  prints("Generating file of size: "); printd(fileSize*BlockSize); prints("\n");
  Create(argv[1]);
  if (fileId == -1) {
    prints("Genfile: Error creating file\n");
    return;
  }
  fileId = Open(argv[1]);
  

  for (i = 0; i < BlockSize; i++) {
    G[i]='G';
  }
  //prints(G);
  for (i = 0; i < fileSize; i++) {
    Write(G , BlockSize ,fileId);
  }

  Close(fileId);
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

int atoi(char s[]) 
{
  int i, n;
  n = 0;
  for (i = 0; s[i] >= '0' && s[i]<= '9'; i++)
    n = 10 * n + (s[i] - '0');
  return n;
}
void prints(char *s) {
  Write(s, strlen(s), 1);
}
