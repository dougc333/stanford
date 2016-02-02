/*
 * File: utiliy.cc
 * ---------------
 * Some of this code is based on code from Eric Roberts' cslib libraries.
 */

#include "utility.h"
#include <stdarg.h>
#include <string.h>
#include "scanner.h"

#define BufferSize   2048
int numErrors = 0;

 
  
void ReportError(struct yyltype *pos, const char *format, ...)
{
  va_list args;
  char errbuf[BufferSize];
  
  numErrors++;
  va_start(args, format);
  vsprintf(errbuf,format, args);
  va_end(args);
  if (strlen(errbuf) > BufferSize) {
    Failure("Error message too long\n");
  } else {
    fflush(stdout); // make sure any buffered text has been output
    if (pos) {
      fprintf(stderr,"\n*** Error line %d.\n", pos->first_line);
    }
    fprintf(stderr,"*** %s\n\n", errbuf);
  }
}

void ReportError(int line, const char *format, ...)
{
  va_list args;
  char errbuf[BufferSize];
  
  numErrors++;
  va_start(args, format);
  vsprintf(errbuf,format, args);
  va_end(args);
  if (strlen(errbuf) > BufferSize) {
    Failure("Error message too long\n");
  } else {
    fflush(stdout); // make sure any buffered text has been output
    if (line != -1) fprintf(stderr,"\n*** Error line %d.\n", line);
    fprintf(stderr,"*** %s\n\n", errbuf);
  }
} 




void Failure(const char *format, ...)
{
  va_list args;
  char errbuf[BufferSize];
  
  va_start(args, format);
  vsprintf(errbuf, format, args);
  va_end(args);
  fflush(stdout);
  if (strlen(errbuf) > BufferSize) 
    fprintf(stderr, "\n*** Failure: Failure message too long\n\n");
  else 
    fprintf(stderr,"\n*** Failure: %s\n\n", errbuf);
  exit(1);
}



#define MaxDebugKeys 256

static char *gDebugKeys[MaxDebugKeys];
static int gNumDebugKeys = 0;
static const int NotFound = -1;

static int FindKeyInList(const char *key)
{
  int i;
  for (i = 0; i < gNumDebugKeys; i++) 
    if (strcmp(key, gDebugKeys[i]) == 0)
      return i;
  return NotFound;
}

bool IsDebugOn(const char *key)
{
   return (FindKeyInList(key) != NotFound);
}


void SetDebugForKey(const char *key, bool value)
{
  int pos = FindKeyInList(key);

  if (value) {
    if (pos != NotFound)
	return;
    Assert(gNumDebugKeys < MaxDebugKeys - 1);
    gDebugKeys[gNumDebugKeys++] = strdup(key);
  } else {
    if (pos == NotFound)
	return;
    gDebugKeys[pos] = "";
  }
}



void PrintDebug(const char *key, const char *format, ...)
{
  va_list args;
  char buf[BufferSize];

  if (!IsDebugOn(key))
     return;
  
  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);
  if (strlen(buf) > BufferSize) 
    Failure("PrintDebug message too long\n");
  else
    printf("+++ (%s): %s%s", key, buf, buf[strlen(buf)-1] != '\n'? "\n" : "");
}


void ParseCommandLine(int argc, char *argv[])
{
  if (argc == 1)
    return;
  
  if (strcmp(argv[1], "-d") != 0) { // first arg is not -d
    printf("Usage:   -d <debug-key-1> <debug-key-2> ... \n");
    exit(2);
  }

  for (int i = 2; i < argc; i++)
    SetDebugForKey(argv[i], true);
}




