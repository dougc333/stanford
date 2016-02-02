/* This module demonstrates the construction from a directed multigraph of
   the type defined in graphtrans.h of an NFA transition table with start
   and final states, and will eventually include also a means to produce a
   C document from a regular expression that, when compiled, will provide
   a stack-based NFA or DFA with the appropriate language */

#ifndef DFA_H_
#define DFA_H_

#include "parser.h"
#include "graphtrans.h"
#include "dynarray.h"

#define DFAINITROWS 32

typedef struct NFATableT {
	int start;
	intdynarray finalStates;
	intdynarray **TTable;
	int numstates;
};

typedef struct DFATableT {
	int start;
	intdynarray finalStates;
	intdynarray **NFAstates;
	int *nextstate;
	int numrows;
	int rowsalloc;
};

initNFATable(int nnodes, MGnode *s);
void printNFA(NFATableT *t, char *regexpstr);
void recursiveMGtoNFA(NFATableT *t, MGnode *n);
void convertMGtoNFA(void);
intdynarray *NFATTablead(NFATableT *n, int s, int c);
initDFATable(DFATableT *d);
DFAaddrow(DFATableT *d);
void convertNFAtoDFA(DFATableT *d, NFATableT *n);
void printDFA(DFATableT *d, char *regexpstr);
void NFAreplace(NFATableT *t, int i, int j);
void eliminateNFAdups(NFATableT *t);
void NFAremove(NFATableT *t, int r);
int DFAnextState(DFATableT *d, int state, int symbol);

extern NFATableT NFATable;
extern DFATableT DFATable;

#endif
