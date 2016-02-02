//join test program


#include "syscall.h"
#define BUFSIZE 10
void printd(int n);
void prints(char *s);
int strlen(char *s);
int main(int argc, char *argv[])
{
  int ExitCode, ProcessID;
  prints("\nB is starting C.\n");
  ProcessID = Exec("../test/SUN/testjoinC");
  prints("\nB is joining C.\n");
  ExitCode = Join(ProcessID);
  prints("\nC has exited with exit code:\n");
  printd(ExitCode);
  Exit(2);
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
