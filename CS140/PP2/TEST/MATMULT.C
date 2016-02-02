/* matmult.c 
 *    Test program to do matrix multiplication on large arrays.
 *
 *    Intended to stress virtual memory system.
 *
 *    Ideally, we could read the matrices off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];
void prints(char *s);
int strlen(char *s);
void printd(int n);

int
main()
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A[i][j] = i;
	     B[i][j] = j;
	     C[i][j] = 0;
	}
    prints("A array\n");
    for (i = 0; i < Dim; i++) {
      for (j = 0; j < Dim; j++) {
	printd(A[i][j]);
	prints(" ");
      }
      prints("\n");
    }
    prints("B array\n");
    for (i = 0; i < Dim; i++) {
      for (j = 0; j < Dim; j++) {
	printd(B[i][j]);
	prints(" ");
      }
      prints("\n");
    }
    prints("C array before mult\n");
    for (i = 0; i < Dim; i++) {
      for (j = 0; j < Dim; j++) {
	printd(C[i][j]);
	prints(" ");
      }
      prints("\n");
    }




    for (i = 0; i < Dim; i++)		/* then multiply them together */
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 C[i][j] += A[i][k] * B[k][j];

    prints("C array after mult\n");
    for (i = 0; i < Dim; i++) {
      for (j = 0; j < Dim; j++) {
	printd(C[i][j]);
	prints(" ");
      }
      prints("\n");
    }

    Exit(C[Dim-1][Dim-1]);		/* and then we're done */
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
