//test program for file commands.


#include "syscall.h"
//#include "stdio.h"

int main()
{

  int loop;

  OpenFileId file, file1, file2;

   char* filename;
   char* filename1;
   
   for (loop=0; loop<20000; loop++);
   filename = "fasdfa";
   filename1="dddd";
  
   Create(filename);
   Create(filename1);
   file = Open(filename);
   file1 =Open(filename1);
    //printf(file);

Write(filename, 7, file);
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
