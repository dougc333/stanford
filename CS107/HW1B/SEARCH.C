/* File: search.c
 * ---------------
 * Please be sure to look carefully at the format we ask for it
 * in the handout so that your program operates correctly with regard
 * to our expectation.  It should take one argument, the filename of
 * users, should parse those home pages, and the go into a loop allow
 * the user to do interactive lookups until they quit. The number of
 * matches is printed, then the user has a chance to respond to
 * whether they would like to see the full list. If printed, the
 * full list contains the names in alphabetical order, one per line,
 * each number in ordered.  See the handout or the sample program if
 * you need more guidance.
 *
 * Starting by taking in as much from your HW1a analyze.c program as
 * is useful to you and massage into the new data structures and format.
 */
 
#include <ctype.h>    // for tolower()
#include <stdio.h>   // for printf()

static char safetolower(char ch);
static int StringHash(const char *s, int numBuckets);

	 
int main(int argc, char *argv[])
{
    printf("Hello world!\n");
    return 0;// return 0 by convention to show everything okay
}

static char safetolower(char ch)
{
  return (isascii(ch) ? tolower(ch) : ch);
}

#define MULTIPLIER -1664117991

/* StringHash
 * ----------
 * This function adapted from Eric Roberts' _The Art and Science of C_
 * It takes a string and uses it to derive a "hash code," which 
 * is an integer in the range [0..NumBuckets-1]. The hash code is computed
 * using a method called "linear congruence." A similar function using this
 * method is described on page 144 of Kernighan and Ritchie. The choice of
 * the value for the Multiplier can have a significant effort on the
 * performance of the algorithm, but not on its correctness.
 * This hash function has the additional feature of being case-insensitive,
 * hashing "ZELENSKI" and "Zelenski" to the same code.
 */
static int StringHash(const char *s, int numBuckets)
{
    int i;
    unsigned long hashcode = 0;
    
    for (i = 0; i < strlen(s); i++)
	  hashcode = hashcode * MULTIPLIER + safetolower(s[i]);
    return (hashcode % numBuckets);
}

