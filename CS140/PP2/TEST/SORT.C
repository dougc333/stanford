/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

#define SORT_SIZE 64

int A[SORT_SIZE];
void prints(char *s);
int strlen(char *s);
void printd(int n);
int
main()
{
    int i, j, tmp;

    prints("Initializing the array\n");
    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < SORT_SIZE; i++) {		
        A[i] = SORT_SIZE - i - 1;
	printd(A[i]);
	prints(" ");
    }
    prints("\n");

    prints("Sorting\n");
    /* then sort! */
    for (i = 0; i < (SORT_SIZE-1); i++)
        for (j = 0; j < (SORT_SIZE - 1 - i); j++)
	   if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = A[j];
	      A[j] = A[j + 1];
	      A[j + 1] = tmp;
    	   }

    for (i = 0; i < SORT_SIZE; i++) {		
	printd(A[i]);
	prints(" ");
    }
    prints("\n");

    prints("Exiting.\n");

    Exit(A[0]);		/* and then we're done -- should be 0! */
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
