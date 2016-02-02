#include "syscall.h"

int
main()
{
  SpaceId newProc;
  OpenFileId input = StandardInput;
  OpenFileId output = StandardOutput;
  char prompt[2], ch, buffer[120];
  int i;
  int joinIt;
  
  
  prompt[0] = '-';
  prompt[1] = '-';
  
  while( 1 )
    {
      Write(prompt, 2, output);
      
      i = 0;
      
      do {
	
	Read(&buffer[i], 1, input); 
	i++;
      } while(( buffer[i-1] != '\n') && (i<120) );
      
      buffer[--i] = '\0';
      
      if (buffer[0]!='&')
	joinIt=1;
      else 
	joinIt=0;
      
      if( i > 0 ) {
	if (joinIt) {
	  newProc = Exec(buffer);
	  Join(newProc);
	}
	else newProc = Exec(buffer+1);
      } 
    }
}

