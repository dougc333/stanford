/* File: adttest.c
 * ---------------
 * These functions are designed to help you test your DynamicArray and
 * HashTable ADTs, as well as show the client usage of these new data types.
 *
 * The DynamicArray test builds an array of characters and appends, inserts,
 * deletes, sorts, and searches the array using the DArray operations.
 *
 * The HashTable test builds a table of letter frequencies by reading the
 * characters from a file, then copies the elements to a DArray and sorts by
 * occurrence and prints out the results.
 *
 * Although not exhaustive, these tests should help you to try out the basic
 * functionality of the ADTs and show you how a client might use these two
 * collection objects.
 */

#include "darray.h"
#include "hashtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <assert.h>

#define YES_OR_NO(value) (value != 0 ? "Yes" : "No")

static void TestCharArray(void);
static void TestHashTable(void);
static void TestCharArray(void);
static void TestArrayAppend(DArray alphabet);
static void TestArraySortSearch(DArray alphabet);
static void TestArrayNth(DArray alphabet);
static void TestArrayInsertDelete(DArray alphabet);
static void TestArrayReplace(DArray alphabet);
static void TestArrayMemory(void);
static void FreeThingElem(void *elemPtr);
static void TestArraySearch(DArray array, char ch);
static int CompareChar(const void *elem1, const void *elem2);
static void PrintChar(void *elem, void *fp);
static void AddFrequency(void *elem, void *array);
static void PrintFrequency(void *elem, void *fp);
static HashTable BuildTableOfLetterCounts(void);
static int CompareLetter(const void *elem1, const void *elem2);
static int CompareOccurrences(const void *elem1, const void *elem2);
static int HashFrequency(const void *elem, int numBuckets);


int main(void) 
{
    TestCharArray();
    TestArrayMemory();
    TestHashTable();	
    return 0;
}



/* =====================   DARRAY TEST   ======================= */


/* TestCharArray
 * -------------
 * Runs a test of the DynamicArray ADT using characters as the element type.
 * Tests append, insert, delete, accessing nth, mapping, sorting, and
 * searching, each in separate functions. Prints results of tests to stdout.
 */
static void TestCharArray(void)
{
    DArray alphabet;
    
    fprintf(stdout, " ------------------------- Starting the DArray test\n");
    alphabet = ArrayNew(sizeof(char), 4, NULL);
    TestArrayAppend(alphabet);
    TestArraySortSearch(alphabet);
    TestArrayNth(alphabet);
    TestArrayInsertDelete(alphabet);
    TestArrayReplace(alphabet);
    ArrayFree(alphabet);			// Free all storage 
}


/* TestArrayAppend
 * ---------------
 * Appends letters of alphabet in order, then appends a few digit chars.
 * Uses ArrayMap to print darray contents before and after.
 */
static void TestArrayAppend(DArray alphabet)
{
    char ch;
    int i;
    
    for (ch = 'A'; ch <= 'Z'; ch++) 	//  Start with letters of alphabet
    	ArrayAppend(alphabet, &ch);
    fprintf(stdout, "First, here is the alphabet: ");
    ArrayMap(alphabet, PrintChar, stdout);
    
    for (i = 0; i < 10; i++) {		// Append digit characters
	   ch = '0' + i;   // convert int to ASCII digit character
	   ArrayAppend(alphabet, &ch);
    }
    fprintf(stdout, "\nAfter append digits: ");
    ArrayMap(alphabet, PrintChar, stdout);
}


/* TestArraySortSearch
 * -------------------
 * Sorts the darray into alphabetic order and then tests searching
 * capabilities, both the linear and binary search versions.
 */
static void TestArraySortSearch(DArray alphabet)
{
    ArraySort(alphabet, CompareChar);	 // Sort into order again
    fprintf(stdout, "\nAfter sorting: ");
    ArrayMap(alphabet, PrintChar, stdout);
    
    TestArraySearch(alphabet, 'J');	// Test searching capabilities
    TestArraySearch(alphabet, '$');
}

/* TestArrayNth
 * -------------
 * Uses ArrayNth to access every other letter and then lowercases
 * it. Prints results using map.
 */
static void TestArrayNth(DArray alphabet)
{
    int i;
    
    for (i = 0; i < ArrayLength(alphabet); i += 2) {// Lowercase every other
         char *elem = (char *)ArrayNth(alphabet, i);
	 *elem = tolower(*elem);
    }
    fprintf(stdout, "\nAfter lowercase every other letter: ");
    ArrayMap(alphabet, PrintChar, stdout);
}


/* TestArrayInsertDelete
 * ---------------------
 * Inserts dashes at regular intervals, then uses delete to remove
 * them. Makes sure that insert at allows you to insert at end of
 * array and checks no problem with deleting very last element. It's
 * always a good idea to directly test the borderline cases to make
 * sure you have handled even the unusual scenarios.
 */
static void TestArrayInsertDelete(DArray alphabet)
{
    int i;
    char ch = '-';
    
    for (i = 3; i < ArrayLength(alphabet); i += 4) // Insert dash every 4th char 
         ArrayInsertAt(alphabet, &ch, i);
    fprintf(stdout, "\nAfter insert dashes: ");
    ArrayMap(alphabet, PrintChar, stdout);

     for (i = 3; i < ArrayLength(alphabet); i += 3) // Delete every 4th char 
         ArrayDeleteAt(alphabet,  i);
    fprintf(stdout, "\nAfter deleting dashes: ");
    ArrayMap(alphabet, PrintChar, stdout);
    
    ch = '!';
    ArrayInsertAt(alphabet, &ch, ArrayLength(alphabet));
    ArrayDeleteAt(alphabet, ArrayLength(alphabet)-1);
    fprintf(stdout, "\nAfter adding and deleting to very end: ");
    ArrayMap(alphabet, PrintChar, stdout);
}

/* TestArrayReplace
 * -----------------
 * Uses repeated searches to find all occurrences of a particular
 * character and then uses replace it to overwrite value.
 */
static void TestArrayReplace(DArray alphabet)
{
    int found = 0;
    char toFind = 's', toReplace = '*';
    
    while (found < ArrayLength(alphabet)) {
    	found = ArraySearch(alphabet, &toFind, CompareChar, found, false);
		if (found == NOT_FOUND) break;
		ArrayReplaceAt(alphabet, &toReplace, found);
    }
    fprintf(stdout, "\nAfter changing all %c to %c: ", toFind, toReplace);
    ArrayMap(alphabet, PrintChar, stdout);
}


/* TestArraySearch
 * ---------------
 * Tests the searching capability of the DArray by looking for specific
 * character. Calls ArraySearch twice, once to see if it finds the character
 * using a binary search (given the array is sorted) and once to see if it
 * finds the character using a linear search. Reports results to stdout.
 */
static void TestArraySearch(DArray array, char ch)
{
    int foundSorted, foundNot;
    
    foundSorted = ArraySearch(array, &ch, CompareChar, 0, true); // Test sorted 
    foundNot = ArraySearch(array, &ch, CompareChar, 0, false);  // Not sorted 
    fprintf(stdout,"\nFound '%c' in sorted array? %s. How about unsorted? %s.", 
    		ch, YES_OR_NO(foundSorted != NOT_FOUND), 
		     YES_OR_NO(foundNot != NOT_FOUND));
}


/* PrintChar
 * ---------
 * Mapping function used to print one character element in a DArray.  The file 
 * pointer is passed as the client data, so that it can be used to print to
 * any FILE *.
 */
static void PrintChar(void *elem, void *fp)
{
     fprintf((FILE *)fp, "%c", *(char *)elem);
}


/* CompareChar
 * -----------
 * Comparator function used to compare two character elements within a DArray.
 * Used for both sorting and searching in the array of characters. Has the same
 * return semantics as strcmp library function (negative if A<B, zero if A==B,
 * positive if A>B).
 */
static int CompareChar(const void *elemA, const void *elemB)
{
    return (*(char *)elemA - *(char *)elemB);
}


#define NUM_THINGS 5

struct thing {
    char *string;
    int length;
};

/* MakeNewThing
 * ------------
 * Makes new struct and makes heap copy of the string to put into it.
 */
static struct thing MakeNewThing(const char *s)
{
    struct thing t;
    t.length = strlen(s);
    t.string = malloc(t.length + 1);
    assert (t.string != NULL);
    strcpy(t.string, s);
    return t;
}

/* FreeThingElem
 * -------------
 * Free function used for a thing struct that is placed in the DArray. Since
 * it has a dynamically allocated pointer, we need the free function to clean
 * up and free that region whenever a thing struct is deleted from the array,
 * or when the entire array is freed.
 */
static void FreeThingElem(void *elemPtr)
{
    free(((struct thing *)elemPtr)->string);
}


/* TestArrayMemory
 * ---------------
 * Runs a test of the DynamicArray ADT to specify see how it handles
 * memory stress, we give a small allocation chunk size, (so it will
 * resize frequently), and make use of the free function to be sure that
 * the Darray is properly dealing with element freeing as needed.
 * Combine this code with Purify's watchful eye for maximal benefit.
 */
static void TestArrayMemory(void)
{
    int i;
    const char *strings[NUM_THINGS] = {"who", "what", "when", "where", "why"};
    DArray things;
    struct thing t;
    
    fprintf(stdout, "\n\n ------------------------- Starting the DArray memory test\n");
    things = ArrayNew(sizeof(struct thing), 1, FreeThingElem);
    
    for (i = 0; i < NUM_THINGS*20; i++) {
	    t = MakeNewThing(strings[i % NUM_THINGS]);
    	ArrayInsertAt(things, &t, 0);	// force to shuffle over a bunch
    }
    for (i = 0; i < 10; i++) {
    	ArrayDeleteAt(things, i*2);	// delete out of middle a few times
    }
    t = MakeNewThing("how");
    ArrayAppend(things, &t);
    ArrayFree(things);			// Free all storage 
    fprintf(stdout, "\nEnd of memory test (but check Purify report to make sure all is well).\n");
}


/* =====================   HASHTABLE TEST   ======================= */

#define NBUCKETS 26 	// only counting the alphabet

struct frequency {
    char ch;		// a particular letter
    int occurrences;	// the number of times it occurs
};


/* TestHashTable
 * -------------
 * Runs a test of the HashTable ADT using a frequency structure as the element 
 * type.  It will open a file, read each char, and count the number of
 * times each char occurs. Tests enter, lookup, and mapping for hashtable. 
 * Prints contents of table to stdout.  Then it dumps all the table elements 
 * into a DynamicArray and sorts them by frequency of occurrences and 
 * prints the array out. Note that as particular stress test it passes
 * 0 as the numElemsToAllocate, which the DArray is required to handle
 * gracefully - be careful!
 */
static void TestHashTable(void)
{
    HashTable counts;
    DArray sortedCounts;
    
    fprintf(stdout, "\n\n ------------------------- Starting the HashTable test\n");
    counts = BuildTableOfLetterCounts();
    
    fprintf(stdout, "Here is the unordered contents of the table:\n");
    TableMap(counts, PrintFrequency, stdout);  // print contents of table
    
    sortedCounts = ArrayNew(sizeof(struct frequency), 0, NULL);
    TableMap(counts, AddFrequency, sortedCounts);   // add all freq to array
    ArraySort(sortedCounts, CompareChar);      // sort by char
    fprintf(stdout, "\nHere are the trials sorted by char: \n");
    ArrayMap(sortedCounts, PrintFrequency, stdout);
    
    ArraySort(sortedCounts, CompareOccurrences); //sort by occurrences
    fprintf(stdout, "\nHere are the trials sorted by occurrence & char: \n");
    ArrayMap(sortedCounts, PrintFrequency, stdout);	// print out array 
    
    ArrayFree(sortedCounts);				// free all storage 
    TableFree(counts);
}


/* BuildTableOfLetterCounts
 * ------------------------
 * Opens a file (in this case, the adttest.c you are currently reading).
 * and pulls characters out one by one and counts the number of times
 * each character is using a hashtable to store structs with the letter
 * and the number of occurrences.
 */
static HashTable BuildTableOfLetterCounts(void)
{
    HashTable counts;
    struct frequency localFreq, *found;
    int ch;
    FILE *fp = fopen("adttest.c", "r"); // open self as file
   
    assert(fp != NULL);
    counts = TableNew(sizeof(struct frequency), NBUCKETS, 
    			HashFrequency, CompareLetter, NULL);
			
    while ((ch = getc(fp)) != EOF) {
	  if (isalpha(ch)) { // only count letters
	    localFreq.ch = tolower(ch);
	    localFreq.occurrences = 1;
	    
			// See if we already have an entry for this char		
	    found = (struct frequency *)TableLookup(counts, &localFreq);	
	    if (found != NULL) 		// increment if already there 
		  localFreq.occurrences = found->occurrences + 1;
	    TableEnter(counts, &localFreq); // enter should overwrite if needed
	  }
    }
    fclose(fp);
    return counts;
}


/* HashFrequency
 * -------------
 * Hash function used to partition frequency structures into buckets.  Our
 * hash function is pretty simplistic, we simply mod the char by the
 * number of buckets.  This provides a number in the range 0 to numBuckets -1,
 * which is the proper range for our table.
 */
static int HashFrequency(const void *elem, int numBuckets)
{
    struct frequency *freq = (struct frequency *)elem;
    return (freq->ch % numBuckets);
}


/* AddFrequency
 * --------------
 * Mapping function used to take a frequency stored in a HashTable and append
 * it to an DArray of frequencies.  The DArray is passed as the client data.
 */
static void AddFrequency(void *elem, void *array)
{
     ArrayAppend((DArray)array, elem);
}

/* PrintFrequency
 * --------------
 * Mapping function used to print one frequency stored in a HashTable.  The
 * file is passed as the client data, so that it can be used to print to
 * any FILE *.
 */
static void PrintFrequency(void *elem, void *fp)
{
     struct frequency *freq = (struct frequency *)elem;
     fprintf((FILE *)fp, "Character %c occurred %4d times\n", 
     		freq->ch, freq->occurrences);
}

/* CompareChar
 * -------------
 * Comparator function used to compare two frequency entries within a table.
 * Two frequencies are considered the same if they refer to the same char.
 * This function is used in TableLookup to decide if a char already has been
 * entered. This function has the same return semantics as strcmp: 
 * (negative if 1<2, zero if 1==2, positive if 1>2).
 */
static int CompareLetter(const void *elem1, const void *elem2)
{
    struct frequency *freq1 = (struct frequency *)elem1;
    struct frequency *freq2 = (struct frequency *)elem2;
    
    return (freq1->ch - freq2->ch);
}

/* CompareOccurrences
 * ------------------
 * Comparator function used to compare two frequency entries.  This one sorts
 * by occurrence as the primary key, and if occurrences are the same, it uses
 * the char as the secondary key, thus the call to CompareLetter which
 * compares the chars. It "reverse-sorts" so a higher frequency bubbles to
 * the top.
 */
static int CompareOccurrences(const void *elem1, const void *elem2)
{
    struct frequency *freq1 = (struct frequency *)elem1;
    struct frequency *freq2 = (struct frequency *)elem2;
    
    if (freq1->occurrences > freq2->occurrences) return -1; 
    else if (freq1->occurrences < freq2->occurrences) return 1;
    else return CompareLetter(freq1, freq2);
}

