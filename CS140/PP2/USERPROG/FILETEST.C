//test program for file commands.


#include "syscall.h"



int main( int argc, char *argv[] )
{
  
  OpenFileId file1, file2;

  char buffer[100];

  file1=Open( argv[1] );

  Create( argv[2] );

  file2=Open( argv[2] );
  
  Read( buffer, 100, file1 );

  Write ( buffer, 100, file2 );


  Close( file1 );

  Close ( file2 );
  

}
