//test program for file commands.


#include "syscall.h"
//#include "stdio.h"

void prints(char *s);
int strlen(char *s);
void printd(int n);

int main()
{

  int loop;

  OpenFileId dougf, eliasf, lionelf, testf;
  int dougs, eliass, lionels, tests;

  // three files easy to recognise
   char* doug;
   char* elias;
   char* lionel, test;

   // A little buffer
   //char buf[1000000];

   // Testing Create
   // Creating a file with an empty name
prints("\nCreating a file with an empty name.\n");
   Create ("");
   // Buffer overflow
   // The shell only take 60 characters, so we use 120
prints("\nCreating a file with a name of 120 characters.\n");
   Create ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
prints("\nWe try too create a file with a 120 character name, the name is truncated to 60.\n");
   // Creating our three cool files
prints("\nCreating three empty files.\n");
   Create("doug");
   Create("elias");
   Create("lionel");

   // Testing Open
   // Opening a wrong file name
   // Empty name
prints("\nOpening a file with an empty name.\n");
   dougf = Open("");
   // Uncreated file
prints("\nOpening a file that does not exit.\n");
   dougf = Open("Nodoug");
   // The shell only take 60 characters, so we use 120
prints("\nOpening a file with a name of 120 characters.\n");
   dougf = Open("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
   // Normal usage
   dougf = Open("doug");
   eliasf = Open("elias");
   lionelf = Open("lionel");
   
   // Testing Close
   // Closing a wrong file name
   // Empty ID
   //Close(NULL);
   // Negative ID
prints("\nClosing fileID -1.\n");
   Close(-2);
   // Standard Input
prints("\nClosing standard input.\n");
   Close(0);
   // Standard Output
prints("\nClosing standard output.\n");
   Close(1);
   // Buffer overflow
prints("\nClosing 99999999999999999999999999999999.\n");
   Close(99999999999999999999999999999999);
   // Unopened File
   Close(25);

   // Normal usage
   Close(dougf);
   Close(eliasf);
   Close(lionelf);

   doug = "Doug est content!"; 

   // Testing Write
   // Unopened file
prints("\nWriting an Unopened File.\n");
   Write(doug, 18, dougf);
   dougf = Open("doug");
   // Uninitialised buffer
prints("\nWriting an Uninitialised buffer.\n");
   Write(lionel, 4, dougf);
   // Uninitialised size
prints("\nWriting an Uninitialised size.\n");
   Write(doug, eliass, dougf);
   // Uninitialised FileID
prints("\nWriting an Uninit FileID.\n");
   Write(doug, 18, testf);
   // Negative file ID
prints("\nWriting an negative FileID.\n");
   Write(doug, 18, -1);
   // Standard input
prints("\nWriting to 0.\n");
   Write(doug, 18, 0);
   // Standard output
prints("\nWriting to 1.\n");
   Write(doug, 18, 1);
   // buffer size < size
prints("\nWriting to buffer size < size.\n");
   Write(doug, 35, dougf);
prints(doug);
   // buffer size > size
prints("\nWriting to buffer size > size.\n");
   Write(doug, 4, dougf);
   // buffer size > main memory
   //Write(buf, 400000, dougf);
   // buffer size = 0
   Write(doug, 0, dougf);
   // normal usage
prints("\nnormal writes\n");
   elias = "Elias est content!";
   lionel = "Lionel est content";
   Write(doug, 18, dougf);
   Write(elias, 19, eliasf);
   Write(lionel, 20, lionelf);
   
   // Testing Read
   // Unopened file
   Write("Unopened\0", 9, dougf); 
   Close(dougf);
prints("\nReading an unopened file.\n");
   Read(doug, 18, dougf);
prints(doug);
   dougf = Open("doug");
   // Uninitialised buffer
prints("\nReading an uninitialised buffer.\n");
   Read(test, 4, dougf);
   // Uninitialised size
prints("\nReading an uninitialised size.\n");
   Read(doug, tests, dougf);
   doug = "Doug est content!\0"; 
   // Uninitialised FileID
prints("\nReading an uninitialised FileID.\n");
printd(&testf);
prints("\nContent of Doug\n");
prints(doug);
//why does the system hang here?
   Read(doug, 18, testf);
   doug = "Doug est content!"; 
   // Negative file ID
prints("\nReading a negative file ID.\n");
   Read(doug, 18, -1);
   doug = "Doug est content!"; 
   // Standard input
   //prints("\nReading on the standard input\n");
   //Read(doug, 18, 0);
   doug = "Doug est content!"; 
   // Standard output
   //prints("\nReading on the standard output\n");
   //Read(doug, 18, 1);
   doug = "Doug est content!"; 
   // buffer size < size
prints("\nReading a buffer size < size.\n");
   Read(doug, 35, dougf);
   doug = "Doug est content!"; 
   // buffer size > size
prints("\nReading a buffer size > size.\n");
   Read(doug, 4, dougf);
   doug = "Doug est content!"; 
   // buffer size > main memory
   //Read(buf, 400000, dougf);
   doug = "Doug est content!"; 
   // buffer size = 0
prints("\nReading a buffer size = 0.\n");
   Read(doug, 0, dougf);
   doug = "Doug est content!"; 
   // normal usage
prints("\nReading normally.\n");
   Read(doug, 18, dougf);
   Read(elias, 19, eliasf);
   Read(lionel, 20, lionelf);
 
   Exit(0);
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
