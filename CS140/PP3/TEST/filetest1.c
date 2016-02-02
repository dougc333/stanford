//test program for file commands.


#include "syscall.h"
//#include "stdio.h"

int main()
{

  int loop;

  OpenFileId file, file1, file2;

   char* filename;
   char* filename1;
   char* filetest2;

 
   for (loop=0; loop<20000; loop++);
   filename = "\0";
   //filename='\0';
   filename1="dddd";
   filetest2="mama";

   Create(filename);
   Create(filename1);
   file = Open(filename);
   file1 =Open(filename1);
    //printf(file);

   //
   //test error condition, what happens when writing a file that is not open
   
   Write(filetest2, 7, file);

   //normal writes
   Write(filename,  7, file);
   Write(filename1, 5, file1);
   
   Close(file);
   Close(file1);
//file = Open(filename);
//file1 = Open(filename1);
//Read(filename, 4, file1);
//Write(filename, 5, file);
//Close(file);

//Write(filename, 5, 1);
//Read(filename, 7, 0);
//Write(filename, 7, file1);
//Close(file1);
     //Write ( filename, 6, file );
 
}
