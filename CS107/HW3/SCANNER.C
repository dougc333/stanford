#include "scanner.h"
#include <assert.h> // for assert macro
#include <stdlib.h> // for malloc
#include <stdio.h> // for print functions
#include <string.h> // for strchr, strdup

/* Type: ScannerImplementation
 * ---------------------------
 * This structure includes a string of delimiters, a pointer
 * to the input file, and two booleans. DiscardDelimiter decides
 * whether or not delimiter should be treated as tokens, if true
 * delimiters are discarded. ScannerOpenedFile keeps track of
 * whether the client or the scanner opened the file (for later
 * free scanning purposes)
 */

struct ScannerImplementation
{
  char* delimiterSet;
  FILE* filePtr;
  bool discardDelimiter;
  bool ScannerOpenedFile;
};

Scanner NewScannerFromFilename(const char *filename, const char *delimiters,
						     bool discardDelimiters) 
{
  Scanner newscan; 
  FILE *infile;
  infile = fopen(filename, "r");
  newscan = NewScannerFromFile(infile, delimiters, discardDelimiters);
  if (newscan != NULL)
     {
     newscan->ScannerOpenedFile = true;
     return newscan;
     }

  return NULL; // here so client won't crash
  
}

Scanner NewScannerFromFile(FILE *fp, const char *delimiters,
				     bool discardDelimiters) 
{
  if (fp != NULL)
     {
     Scanner newscan;
     newscan = malloc(sizeof(struct ScannerImplementation));
     assert(newscan != NULL);
     newscan->delimiterSet = strdup(delimiters);
     if (newscan->delimiterSet == NULL)
       { 
        printf("No memory available");
        return NULL;
       }
     newscan->filePtr = fp;
     newscan->discardDelimiter = discardDelimiters;
     newscan->ScannerOpenedFile = false; // indicates that client opened file
     return newscan;
     } 
  
   return NULL; // this is here so the client won't crash
}



void FreeScanner(Scanner s)
{
  assert(s != NULL);

  if (s->ScannerOpenedFile) // Scanner created using NewScannerFromFilename
     fclose(s->filePtr);
  free(s->delimiterSet);
  free(s);

}


bool ReadNextToken(Scanner s, char buffer[], int bufLen) 
{
  int ch;
  
  int count = 0;
  assert(bufLen >=2);
 
  while ((ch = getc(s->filePtr)) != EOF)
        {
	if (strchr(s->delimiterSet, ch) == NULL) // ch is not a delimiting
						 // char
	   {
	   if (count < bufLen)  // ensures that length is within bufLen 
              {
              buffer[count] = ch;
              count++;
              }
           else
              {
	      printf("The token exceeded the buffer length, and was truncated.\n");
              break;
              }   
           }
        else   // specified character IS a delimiter
	   {
	   if (s->discardDelimiter == false)
	      { 
	      if (count == 0)
                 { 
                 // the first delimiter must be put into the buffer
	         buffer[count] = ch;
                 buffer[count + 1] = 0; // We are guaranteed that the
		                        // length is greater than 2 by assert
                 return true;
	         } 
              else 
		 {
		 ungetc(ch, s->filePtr); // Need to put delimiter back on
		                         // stream so that it can be read
		                         // and tokenized on the
		                         // next function call
                 buffer[count] = 0;      // To become a NULL-terminated string
                 return true;
                 }
              }
           else if ((s->discardDelimiter) && (count != 0))
	       {
               break;
               }
             }
	}
    
    /* If the code reaches this point, one of three things may have
       happened: a) ch == EOF and nothing was added to the buffer, b)  ch == EOF
       and characters were added to the buffer, or c) ch != EOF and
       characters were added to the buffer and the while loop met a break
       statement. In both cases b & c, we are guaranteed that count > 0,
       whereas count = 0 in case a. Hence the following comparison. */
   
    if (count > 0)
       { 
       buffer[count] = 0; // Append NULL to end of buffer
       return true;
       } 
    return false;
}

int SkipHelper(Scanner s, const char *Set, bool untilIndicator)
{
  int ch;
  assert(Set != NULL);
  
  if (untilIndicator)
     {
     while ((ch = getc(s->filePtr)) != EOF && strchr(Set, ch) == NULL)
	   {
	   // Runs through input until a character in untilSet is found
           }
     }
  else
     {
     while ((ch = getc(s->filePtr)) != EOF && strchr(Set, ch) != NULL)
	   {
	   // Runs through input while the characters consist of SkipSet values
           }
     }
   
  if (ch == EOF) // reached end without finding any non-SkipSet values
     return EOF;

  ungetc(ch, s->filePtr); // The halting character pushed back on the stream
  
  return ch;
}
  
     
int SkipOver(Scanner s, const char *skipSet) 
{
   return SkipHelper(s, skipSet, false);
}

int SkipUntil(Scanner s, const char *untilSet) 
{
   return SkipHelper(s, untilSet, true);
}
