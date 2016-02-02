#include "syscall.h"


int
main()
{
  int openFileId;
  char buffer[100];
  int i;
  int size = 100;

  for (i=0; i<size; i++) {
    buffer[i] = 'i';
  }

  Create("testFile");
  openFileId = Open("testFile");
  Write(buffer, size, openFileId);
  Close(openFileId);
  
  /*
  openFileId = Open("test.c");
  size = Read(buffer, size, openFileId);
  Close(openFileId);

  for(i=0; i<size; i++) {
    printf("%d ", buffer[i]);
  }
*/
    /* not reached */
  
  Exit(0);
}
