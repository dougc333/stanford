//word count test program.


#include "syscall.h"

//#include "stdio.h"
void printd(int n);
void prints(char *s);
int strlen(char *s);

int main(int argc, char *argv[])
{
  typedef int SegmentId;
  OpenFileId fid;
  int length=0;
  int length1=0;
  SegmentId sid;
  int i;
  
  //printd(1);
  char *addr;
char *Buffer;
  //addr = new char[20000];
  addr = (char *) 0x00004000;
  fid = Open("../test/smallfile");
  prints("\nfid:");
  printd(fid);
  //printd(2);
  length1 = Length(fid);
  //printd(3);
  // for testing, make the length multiple pages
  //length =10000;
  prints("\nLenght:");
  printd(length1);
  prints("\n");
	 
  sid  = Mmap(fid, addr, length1);
  //Read(Buffer, 9, sid);
  //prints(addr[0]);
  prints("Memory Address is:");
  printd((int) addr);
  prints("\n");
  Write(addr, 3, StandardOutput);
  //Munmap(sid);
  //Write(addr, 64, StandardOutput);
   // printd(sid);
prints("\nMemory Address is:");
  printd((int) addr);
  prints("\n");

  for (i=4000; i<4600; i++)
    addr[i]='X';
  Write(addr, 10, StandardOutput);
  
  if(sid != -1) 
  Munmap(sid);
  //Write(addr, 3, StandardOutput);
  //add some illegal conditions for Munmap
  //open an nonexistent segment
  //Munmap(100);
  //unmap something already unmapped
  //Munmap(sid);
  if (fid!=-1)
    Close(fid);
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
