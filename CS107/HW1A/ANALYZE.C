/* File: analyze.c  
 * ---------------
 * The great leland homepage word frequency analyzer!  Rather than give
 * you a big blank page to stare at, we've given you a good starting
 * point. Below is a working program that will extract the words
 * words from one home page, unique them, and list in alphabetical  
 * order. When you run the program, it takes one argument which is the
 * leland user whose page you would like analyzed.   
 * 
 * It will be your job to modify this program to keep frequency counts 
 * and to analyze an entire collection of pages, not just one.
 *
 * Once you have implemented your scanner, you can first use this client 
 * program without modification to test it. Once you're satisfied that 
 * your scanner is working correctly, you're ready to tackle modifying 
 * this program-- probably first by adding frequency counts, and then
 * onto analyzing a whole set of pages and reporting their amassed 
 * frequency counts. Your modified program will take 1 argument from
 * the shell, which is the filename of the file containing the
 * usernames to consider, rather than the name of a single user to analyze.
 *
 * Be sure that your final output is listed in order of decreasing 
 * frequency and that each line contains just the frequency count and
 * the word in order to make it easy for us to verify that your solution 
 * is working correctly.
 *
 * Because we have given you a bunch of the code already written,
 * there is less for you to implement (always a plus :-), but
 * further, this code also serves as an example of the level of 
 * decomposition, commenting, and attention to detail we are expecting 
 * from your submissions. Please do pay attention to these issues-- 
 * design and readability will count for about half of your grade.
 */

#include "scanner.h" 
#include <assert.h>
#include <stdlib.h>     // for malloc 
#include <string.h>     // for strcasecmp
#include <limits.h>     // for PATH_MAX  
#include <ctype.h>      // for isascii

/* Specifications used for analyzing the home pages. You should
 * not change any of these values.
 */

#define DELIMITERS " \t\n\r.,!\"(){};:/?\\*^%$#@&[]=<>~-`|+<>"
#define MAX_WORD_LEN 100
#define MIN_WORD_LEN 4
#define MAX_WORDS_IN_LIST 10000

/* Type: struct wordlist
 * ---------------------
 * This structure is just a simple fixed-size array of strings packaged with
 * its current length (i.e. number of entries in use). Since you always need
 * the two together, it's convenient and tidy to put them in one structure.
 */
struct wordlist {
    char *words[MAX_WORDS_IN_LIST];
    int count;
};

static void AnalyzeUserPage(const char *username);
static void ExtractWordsIntoList(Scanner s, struct wordlist *list);
static void AddWordIfAbsent(const char *word, struct wordlist *list);
static int CaseInsensitiveCompare(const void *entry1, const void *entry2);
static void PrintList(struct wordlist *list);
static void FreeList(struct wordlist *list);
static bool ContainsOnlyAscii(const char *s);
static char *CopyString(const char *s);


/* main
 * ----
 * The prototype for main under UNIX is different than what you have
 * seen previously on the Macintosh.  Because UNIX programs are
 * executed from the shell and often have user-given arguments, there
 * are two parameters to main which allow you to access those arguments.
 * "argv" is an array of strings (char *'s), one entry for each argument,
 * "argc" is the number of elements in that array.  The first argument
 * (argv[0]) is always the name of the program. Since we expect one
 * additional argument which is the username to analyze, we ensure
 * we have at least two entries in the argv array and use argv[1] as
 * the username.
 */

int main(int argc, char *argv[])
{
    if (argc < 2) {  
        printf("\nOOPS!  This program takes one argument which is the user\n");
		printf("whose home page you wish to analyze. Please try again!\n\n");
    } else {
        AnalyzeUserPage(argv[1]);
    }
    return 0;   // return 0 by convention to show everything okay
}



/* AnalyzeUserPage
 * ---------------
 * Given a username, this constructs the path to that user's home
 * page and attempts to open a Scanner for that filename.  Notice
 * the tidy use of sprintf to build the pathname by specifying a
 * basic pattern and filling in the letters and name for this 
 * particular user.  We use the stack for the buffer and the
 * wordlist structure, since space on the stack is quick and cheap
 * (as compared to malloc/free). 
 *
 * We try to create a Scanner and if the call returns null, that 
 * means the file didn't exist or couldn't be opened (usually because 
 * of protection) and we just return. Otherwise, we clear the word 
 * list, extract words into it, print the results, and then clean up. 
 * See the lower level functions for more details. One thing to note
 * is that we do not discard delimiters. That is so that we will
 * get notified when we encounter < and > character tokens. We need
 * this so we can tell when we've hit an html tag and quickly skip
 * over all the junk inside without scanning it.
 */

static void AnalyzeUserPage(const char *username)
{
    Scanner s = NULL;
    char filename[PATH_MAX];
    struct wordlist list;
    const char *extensions[] = {"html", "htm", "shtml", "shtm"};
    int numExtensions = sizeof (extensions) / sizeof (extensions[0]);
    int i;
    
    for (i = 0; s == NULL && i < numExtensions; i++) {
      sprintf(filename, "/afs/ir.stanford.edu/users/%c/%c/%s/WWW/index.%s", 
	      username[0], username[1], username, extensions[i]);
      s = NewScannerFromFilename(filename, DELIMITERS, false);
    }
    
    if (s == NULL) {
        printf("\nSorry, there appears to be no page for \"%s\".\n", username);
        return;
    }
    
    printf("\nExtracting words from the home page of \"%s\"...\n", username);
    list.count = 0;
    ExtractWordsIntoList(s, &list);
    PrintList(&list);
    FreeList(&list);
    FreeScanner(s);
}


/* ExtractWordsIntoList
 * --------------------
 * This uses the Scanner to extract the words from the file, by looping
 * calling ReadNextToken until it returns false (which indicates end of
 * file).  If the word is over the minimum length, we add it to the list 
 * if not already present.  Since we know that we are parsing HTML files, 
 * we make a specific effort to exclude tags by checking if the token 
 * begins with '<', and if so, we quickly skip everything up to the 
 * closing '>'. This avoids entering HTML tag words in our list. 
 * We use the stack for the storage for the word (again since the stack
 * is cheap and quick), but if we need to store the word permanently
 * in the list, we have to make a new heap copy, since the stack buffer 
 * will be overwritten each time we read a new token. This copy is made
 * when adding the word to the list (see function AddWordIfAbsent).
 */
static void ExtractWordsIntoList(Scanner s, struct wordlist *list)
{
    char word[MAX_WORD_LEN];

    while (ReadNextToken(s, word, MAX_WORD_LEN)) {
         if (word[0] == '<')     // if HTML opening tag
             SkipUntil(s, ">");  // skip to end of tag
         else if (strlen(word) >= MIN_WORD_LEN && ContainsOnlyAscii(word)) // long enough to list  
             AddWordIfAbsent(word, list);
    } 
}




/* AddWordIfAbsent
 * ---------------
 * Since the wordlist isn't in any particular order, we do a simple
 * slow linear search to determine if the new word is already in the
 * list. If so, we're done. Otherwise, we add the new word onto the 
 * end of the list, making a heap copy that will exist permanently.
 * If all slots in the array are used up, we just ignore the word.
 * (We're just using a simple fixed-size array that doesn't grow).
 */
static void AddWordIfAbsent(const char *word, struct wordlist *list)
{
    int i;
    
    for (i = 0; i < list->count; i++) { 
         if (strcasecmp(word, list->words[i]) == 0) 
              return;                         // found it, nothing else to do
    }
    if (list->count < MAX_WORDS_IN_LIST)
        list->words[list->count++] = CopyString(word);
}



/* PrintList
 * ---------
 * Sorts the list in alphabetical order (case-insensitively) using the
 * standard C library qsort function and then prints the results in
 * one word per line. Later you will sort the list according to
 * frequency count first, then break ties alphabetically and print
 * both the words and frequency count in your results.
 */
static void PrintList(struct wordlist *list)
{
    int i;
    
    qsort(list->words, list->count, sizeof(char *), CaseInsensitiveCompare);
    printf("-----------------------------------------------------\n");
    for (i = 0; i < list->count; i++) 
         printf("\t%s\n", list->words[i]);
}



/* CaseInsensitiveCompare
 * ----------------------
 * Comparison function passed to qsort to sort an array of
 * strings in alphabetical order. It uses strcasecmp which is
 * identical to strcmp, except that it doesn't consider case of the
 * characters when comparing them, thus it sorts case-insensitively.
 */
static int CaseInsensitiveCompare(const void *entry1, const void *entry2)
{
    return strcasecmp(*(char **)entry1,*(char **)entry2);
}



/* FreeList
 * ---------
 * Frees all the dynamically-allocated strings in the word list.
 */
static void FreeList(struct wordlist *list)
{
    int i;
    
    for (i = 0; i < list->count; i++) 
         free(list->words[i]);
}

/* ContainsOnlyAscii
 * -----------------
 * This checks all of the characters in a string to determine if they are
 * plain ASCII characters (as determined by the isascii ctype function)
 * and returns a boolean result of whether all characters are. This
 * allows us to screen out words that contain weird characters that are
 * outside of the standard ASCII character set since those aren't
 * interesting to us.
 */
static bool ContainsOnlyAscii(const char *str)
{
   int i;

   for (i = 0; str[i] != '\0'; i++) {
	 if (!isascii(str[i]))
		 return false;
   }
   return true;
}

/* CopyString
 * ----------
 * Straight-forward utility function to make a new heap copy oof a
 * string. Notice the use of the assert statement after the malloc
 * to raise an immediate error if the allocation request failed for
 * any reason. Defensive programming!
 */
static char *CopyString(const char *s)
{
    char *copy = malloc(strlen(s) + 1);
    assert(copy != NULL);
    strcpy(copy, s);
    return copy;
}







