/* File: swap.c
 * ------------
 * A simple test harness to run some time trials of the hard-coded
 * SwapInt versus generic SwapAny function using mempcy.
 * This is for HW2 CS107.
 *
 * Although we usually give you a Makefile for the multi-module
 * assignments, compiling one file by itself is just a gcc
 * command, so worth learning how to do for later:
 * To compile this program, do this:
 *         gcc swap.c -o swaptest
 * (This says to compile the file swap.c and put the output in an
 * exectuable file called swaptest).  
 * By default, gcc does no optimization and just creates the most
 * straightforward translation of C code to machine code.
 * To compile an optimized version, use gcc O2 flag:
 *          gcc -O2 swap.c -o swaptest
 *
 * When you run the program, you need to give an argument (a string
 * that is either "int"for SwapInt) or "any" for the generic
 * routine) that identifies which swap function to use.
 *         swaptest int
 * To run & report time statistics, use /usr/bin/time, like this:
 *         /usr/bin/time swaptest int
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define NUM_TRIALS 5000000

static void SwapInt(int *a, int *b);
static void SwapFloat(float *a, float *b);
static void SwapChar(char *a, char *b);
static void SwapShort(short *a, short *b);
static void SwapAny(void *a, void *b, int size);

/* In order to get anything measurable at all, we have to call it
 * a lot of times, so we run loop of several million iterations
 * so we have some validity to our numbers.
 */
int main(int argc, char **argv)
{
    int i, nums[2];  
    
    if (argc < 2) {
        printf("This programs needs an argument for which version to use.\n");
        printf("Either int for hard-code or any for generic.\n");
        exit(-1);
    }
    
    if (strcmp(argv[1], "int") == 0) {
	for (i = 0; i < NUM_TRIALS; i++)
     	    SwapInt(&nums[0], &nums[1]);
    } else if (strcmp(argv[1], "any") == 0) {
        for (i = 0; i < NUM_TRIALS; i++)
            SwapAny(&nums[0], &nums[1], sizeof(nums[0]));
    } else {
	  printf("Not a valid choice!  int or any only.\n"); 
	  exit(-1);
    }
    return 0;
}
 
/* SwapInt: just swaps two integers. Nothing more to see here */
static void SwapInt(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

/* SwapAny: can swap any two same-sized values, both passed by
 * address and correct size must be given as third parameter.
 */
static void SwapAny(void *a, void *b, int sz)
{
    void *tmp;

    tmp = malloc(sz);
    assert(tmp != NULL);
    memcpy(tmp, a, sz);
    memcpy(a, b, sz);
    memcpy(b, tmp, sz);
    free(tmp);
}

/* These last few Swap variants aren't used in the performance lab, but 
 * are used later in the HW2 problem set. They all do the obvious
 * exchange based on different variable types.
 */


static void SwapFloat(float *a, float *b)
{
    float tmp = *a;
    *a = *b;
    *b = tmp;
}

static void SwapShort(short *a, short *b)
{
    short tmp = *a;
    *a = *b;
    *b = tmp;
}

static void SwapChar(char *a, char *b)
{
    char tmp = *a;
    *a = *b;
    *b = tmp;
}
