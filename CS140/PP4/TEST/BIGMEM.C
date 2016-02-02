//copy test program


#include "syscall.h"
#define BUFSIZE 10000

void prints(char *s);
int strlen(char *s);
int main(int argc, char *argv[])
{
  char buffer[BUFSIZE];
  long int i;


  for(i=BUFSIZE-1;i>=0; i--)
    buffer[i]='a';
  
  prints("We are loaded in memory.\n");
  buffer[BUFSIZE-6]='E';
  buffer[BUFSIZE-5]='L';
  buffer[BUFSIZE-4]='A';
  buffer[BUFSIZE-3]='I';
  buffer[BUFSIZE-2]='S';
  buffer[BUFSIZE-1]='\0';
  
  //prints(buffer);


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

