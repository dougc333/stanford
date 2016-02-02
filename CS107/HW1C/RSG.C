#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>  /* for INT_MAX, which is 2^31 - 1, max val for random() */

static void PrintWithWrap(FILE *out, const char *format, ...) ;
static void Randomize();
static int RandomInteger(int low, int high);

int main(int argc, char *argv[])
{
    int i;

    Randomize();
    PrintWithWrap(stdout, "Here is a simple program that uses PrintWithWrap. ");
    PrintWithWrap(stdout, "It makes multiple calls to PrintWithWap with long strings and never outputs an explicit newline. ");
    PrintWithWrap(stdout, "However, when you run, the output is wrapped at the right margin! ");
    PrintWithWrap(stdout, "Also note it accepts printf-style formats %d %-5d %s %2.3f ", 1, 45, "Hi!", 3.14159);
    PrintWithWrap(stdout, "Here are some random numbers: ");
    for (i = 0; i < 10; i++)
        PrintWithWrap(stdout, "%d ", RandomInteger(1, 100));
    PrintWithWrap(stdout, "All done!\n");
    return 0;
}


/* 
 * PrintWithWrap
 * -------------
 * This is structured like an fprintf function, you pass the FILE
 * to write to, the format string, and then any number of matchings
 * arguments for the format string.  The only difference is that it
 * internally tracks how many characters are on the current line and
 * when getting to close to the end, finds a convenient space character
 * and turns it into a newline to get new line-wrapping output.
 */
static void PrintWithWrap(FILE *out, const char *format, ...) 
{
    #define WRAP_AT_COLUMN 65
    static int column = 0;  // keeps track of how many characters on this line
    char buffer[2048];
    int i, len;
    va_list ap;

    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);
    len = strlen(buffer);
    
    for (i = 0; i < len; i++) {
	  if ((column > WRAP_AT_COLUMN) && isspace(((int)buffer[i]))) {
         fprintf(out, "\n");		// change space to return 
         column = 0;			// re-start counter
      } else {	// otherwise just print it normally and
           		// keep our internal column number in sync
        fputc(buffer[i], out);
        column++;
        if (buffer[i] == '\n') column = 0;
      }
   }
}



/*
 * RandomInteger
 * -------------
 * Simple random number generator function that returns a random integer in
 * the range low to high inclusive. Algorithm uses same steps you saw back in CS106A.
 * Take result from random, normalize to a number in [0, 1.0). Multiply
 * percentage to scale to range, translate, and truncate to integer.
 * Be sure to the read the FAQ about an interaction between Purify and
 * random() that you will want to know about. Also, don't forget to seed
 * the random number generator (see the Randomize function below).
 */
static int RandomInteger(int low, int high)
{
    int range;
    double percentage;

    assert (high >= low);  /* don't want a negative range! */
    range = high - low + 1;
    percentage = random()/((double)INT_MAX + 1); /* percentage is 0.0 to almost, but never exactly 1.0 */
    return (int)(low + percentage*range);
}


/* Randomize
 * ---------
 * Seeds the random number generator. The time() function returns the
 * current time in seconds, which varies between runs, giving you a
 * new starting point for the random sequence each time.
 */
static void Randomize()
{
    srandom(time(NULL));
}
