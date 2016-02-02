#ifndef _SCANNER_H
#define _SCANNER_H

#include <stdio.h>

/* bool type
 * ---------
 * A true boolean type is a little detail sorely missed in C. Here's a 
 * handy typedef for a simple enum version of it. This is just like the
 * one you used back in CS106.
 */
typedef enum {false, true} bool;

/*
 * Scanner type
 * ------------
 * The Scanner ADT is a simple token scanner that provides the ability to
 * divide up a file token by token.  The client can declare variables of 
 * type Scanner, but they must be initialized with NewScannerFromFilename or 
 * NewScannerFromFile. The Scanner is implemented with pointers, so all client 
 * copies in variables or parameters will be "shallow"- they will all 
 * actually point to the same Scanner structure.  Note that the Scanner is
 * defined as an incomplete type, a pointer to a struct that will only
 * be detailed in the scanner.c file, far from the prying eyes of any
 * Scanner client trying to break the abstraction wall.
 */
typedef struct ScannerImplementation *Scanner; 

/*
 * NewScannerFromFilename
 * ----------------------
 * Allocate and return a new Scanner. The specified filename specifies the
 * file to open for scanning.  If the filename is NULL or the named file 
 * doesn't exist or can't be opened, a NULL scanner is returned.  The 
 * delimiter string lists the characters to be treated as token delimiters. 
 * The boolean discardDelimiters controls whether delimiters are returned as 
 * tokens. (see more info about delimiters in the spec for the ReadNextToken 
 * function). A copy of the delimiter string should be made and kept by the 
 * scanner. (i.e. you should not just store the pointer since you don't know 
 * what will happen to that space later.) There is no limit on the number of 
 * characters that the client may give in the delimiter set, so the scanner 
 * should not make assumptions that introduce any fixed bound. If the  
 * delimiter parameter is NULL, an assert is raised.
 */
Scanner NewScannerFromFilename(const char *filename, const char *delimiters,
			       bool discardDelimiters);

/*
 * NewScannerFromFile
 * ------------------
 * Like the creation function above, but used to scan a file which has already
 * been opened. This is needed when scanning pre-opened files such as stdin.
 * If a null file pointer is passed, a NULL scanner is returned.  See docs
 * for NewScannerFromFilename above for information about delimiters.
 */
Scanner NewScannerFromFile(FILE *fp, const char *delimiters, 
			   bool discardDelimiters);

/*
 * FreeScanner
 * -----------
 * Free all the storage for the given scanner. If the scanner was the one
 * to open the file (i.e. it was created via NewScannerFromFilename not
 * NewScannerFromFile), it also closes the underlying file. (It would not
 * be appropriate for the scanner to close a file that itself did not open.)
 * It is an error for the client to continue to use a scanner after it has
 * FreeScanner has been called on it.
 */
void FreeScanner(Scanner s);

/* ReadNextToken
 * -------------
 * Scans characters from the file to form a string in the client-supplied
 * char buffer. The client must pass a buffer which is valid storage and 
 * gives the length of the buffer in the bufLen parameter. This function
 * extracts the next token from the scanner and writes it into the buffer.
 * The token written into the buffer is null-terminated by this function.
 *
 * Return value: 
 * This function returns a boolean result which indicates whether anything was
 * written into the buffer. If a valid (i.e. non-empty) token was found
 * and written into the client's buffer, the function returns true. If
 * the scanner is at EOF and there are no more tokens to read (or if the
 * the only characters left are delimiters and they are being discarded),
 * it will return false.  Think of the boolean result as communicating to
 * the client whether there was a token written into the buffer that needs to
 * be processed. If the scanner writes a token, it returns true, false 
 * otherwise. Note that if the scanner reads the last token and stops at EOF
 * it still returns true (since a token was written to buffer), but the next
 * call to ReadNextToken will return false (since there are no tokens left).
 *
 * How it forms tokens:
 * ReadNextToken starts by examining the next character in the file.  If
 * the char is a member of the delimiter set, a single-character string 
 * of just that delimiter will be formed. If discardDelimiters is false,
 * it returns it. If discardDelimiters is true, the delimiter token is 
 * discarded and it continues on looking for a non-delimiter token. When a 
 * non-delimiter character is found, it keeps scanning characters and 
 * accumulating a multi-character token into the client buffer until the 
 * first delimiter character (or EOF) is seen. The stopping delimiter 
 * char is left in the input stream for next time.
 *
 * What happens when client buffer is full:
 * The client provides the buffer and indicates how big it is. If while
 * reading a token, the scanner exhausts the buffer before end of the token,
 * it truncates to what fits in the client buffer (leaving space to null-
 * terminate). The next call to ReadNextToken will pick up where it left off.
 * Thus, long tokens are chopped into pieces. No error message is printed or
 * assert raised when this happens. For example, if chars coming are
 * "antidisestablishmentarianism" and the clients scans into a 10-char buffer,
 * it would first pull out "antidises", then "tablishme", etc.
 *
 * Error conditions:
 * If the buflen parameter is less than 2, an assert is raised.
 */
bool ReadNextToken(Scanner s, char buffer[], int bufLen);

/* SkipOver
 * --------
 * Skip over characters in the file so long as they occur in skipSet. For
 * example, to skip over the common white-space characters, pass skipSet 
 * as " \n\r\t" -- the function should keep reading characters as long
 * as they are white-space and stop at the first non white-space character.
 * The character that halted the skipping is left in the input stream so
 * that it will be the very next character read (i.e. it is not skipped).
 * This character is returned as the function result in case the caller 
 * wants to know about it. Hitting the end of file also stops the skipping 
 * and returns the value EOF. An assert is raised if skipSet is NULL.
 */
int SkipOver(Scanner s, const char *skipSet);


/*
 * SkipUntil
 * ---------
 * Like SkipOver, but it skips UNTIL it gets to a character in the untilSet 
 * For example, you could use SkipOver to jump to the end of the sentence
 * by using the untilSet of ".!?" which would scan over characters until it
 * encountered the first period, exclamation or question mark. As with
 * SkipOver, the halting character is left in the input stream and returned
 * as the result from the function.  Again, it will also halt and return EOF
 * if it encounters the end of file. An assert is raised if untilSet is NULL.
 */
int SkipUntil(Scanner s, const char *untilSet);

#endif _SCANNER_H
