#include "syscall.h"

char elias[]="Elias";
int strcmp(char *s, char *t);
void strcpy(char *s, char *t, char token, int offset);
void prints(char *s);
int strlen(char *s);
int
main()
{
  SpaceId newProc;
  OpenFileId input = StandardInput;
  OpenFileId output = StandardOutput;
  char prompt[2], ch, buffer[120], shellCmdBuffer[120];
  int i;
  int joinIt;
  
  prints("\n** Type 'help' at the prompt to see a list of commands\n");
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
      strcpy(shellCmdBuffer, buffer, ' ', 0);
      if( i > 0 ) {
	if (!strcmp(shellCmdBuffer, "halt")) {
	  prints("Thank you for using Screwed OS 2000\n");
	  Halt();
	} else if (!strcmp(shellCmdBuffer, "cd")) {
	  strcpy(shellCmdBuffer, buffer, '\0', 3);
	  Chdir(shellCmdBuffer);
	} else if (!strcmp(shellCmdBuffer, "mkdir")) {
	  strcpy(shellCmdBuffer, buffer, '\0', 6);
	  Mkdir(shellCmdBuffer);
	} else if (!strcmp(shellCmdBuffer, "rm")) {
	  strcpy(shellCmdBuffer, buffer, '\0', 3);
	  Remove(shellCmdBuffer);
	} else if (!strcmp(shellCmdBuffer, "ls")) {
	  Lsdir();
	} else if (!strcmp(shellCmdBuffer, "help")) {
	  prints("Shell Commands:\n");
	  prints("cd\n");
	  prints("mkdir\n");
	  prints("rm\n");
	  prints("ls\n");
	  prints("exit\n");
	} else if (!strcmp(shellCmdBuffer, "exit")) {
          Exit(0);
	} else if (joinIt) {
	  newProc = Exec(buffer);
	  Join(newProc);
	} else newProc = Exec(buffer+1);
      } 
    }
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

int strcmp(char *s, char *t) {
  for ( ; *s == *t; s++, t++)
    if (*s == '\0')
      return 0;
  return *s - *t;
}

void strcpy(char *s, char *t, char token, int offset) {
  int i;
  for (i = 0; i < offset; i++)
    t++;;
  while (*t != token && *t != '\0' ) {
    *s = *t;
    s++;
    t++;
  }
  //s++;
  *s = '\0';
}
