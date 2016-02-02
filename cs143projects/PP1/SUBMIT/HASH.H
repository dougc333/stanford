/* hash table taken from Eric Roberts cs106b book
 * 
 */


#include <string.h>
#include <stdio.h>
#include "utility.h"



/*
 * Type: symtabADT
 * --------------------------------------------------------
 * Thsi is the adt ER uses to represent the hash table
 */
typedef struct symtabCDT *symtabADT;

/* 
 * Function: NewSymbolTable
 * ---------------------------------------------------------
 * this function allocates a new symbol table with no entries
 */
symtabADT NewSymbolTable(void);


/* 
 * Function: FreeSymbolTable
 * ---------------------------------------------------------
 * this function frees the symbol table
 */
void FreeSymbolTable(symtabADT table);



/* 
 * Function: Enter
 * ---------------------------------------------------------
 * this function adds a key,value pair to the hashtable
 */
void Enter(symtabADT table, char *key, char *value);

/* 
 * Function: ChangeValue
 * ---------------------------------------------------------
 * this function changes the value of an existing key
 */

void ChangeValue(symtabADT table, char *key, char *value);

/* 
 * Function: Lookup
 * ---------------------------------------------------------
 * this function searches for a particular key; it returns a 
 * null if the key is not in z table. If the key is found, the 
 * value is returned. 
 */
char *Lookup(symtabADT table, char *key);


