//
// dfa.c
//
// code to support NFA and DFAs for project 1
//

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "dfa.h"

NFATableT NFATable;
DFATableT DFATable;

intdynarray *NFATTablead(NFATableT *n, int s, int c)
{
	return (n->TTable[(s*256)+c]);
}

initNFATable(int nnodes, MGnode *s)
{
	int i, j;
	
	NFATable.start = s->id;
	NFATable.numstates = nnodes;
	NFATable.TTable = (intdynarray **) malloc(nnodes*256*sizeof(intdynarray *));
	for (i=0; i<nnodes; i++)
		for (j=0; j<256; j++)
			NFATable.TTable[(i*256)+j] = NULL;
}

initDFATable(DFATableT *d)
{
	int i, j;
	
	d->numrows = 0;
	d->rowsalloc = DFAINITROWS;
	d->NFAstates = (intdynarray **) malloc(d->rowsalloc*sizeof(intdynarray *));
	for (i=0; i<d->rowsalloc; i++)
		d->NFAstates[i] = new intdynarray();
	d->nextstate = (int *) malloc(d->rowsalloc*256*sizeof(int));
	for (i=0; i<d->rowsalloc; i++)
		for (j=0; j<256; j++)
			d->nextstate[i*256+j] = -1;
}

int DFAaddrow(DFATableT *d)
{
	int i, j;
	int start;
	
	if (d->numrows >= d->rowsalloc)
	{
		start = d->rowsalloc;
		d->rowsalloc *= 2;
		d->NFAstates = (intdynarray **) realloc(d->NFAstates, d->rowsalloc*sizeof(intdynarray *));
		if (!d->NFAstates)
			perror("realloc failed for NFAstates");
		for (i=start; i<d->rowsalloc; i++)
			d->NFAstates[i] = new intdynarray();
		d->nextstate = (int *) realloc(d->nextstate, d->rowsalloc*256*sizeof(int));
		if (!d->nextstate)
			perror("realloc failed for nextstate");
		for (i=start; i<d->rowsalloc; i++)
			for (j=0; j<256; j++)
				d->nextstate[i*256+j] = -1;
	}
	d->numrows++;
	return (d->numrows-1);
}

void recursiveMGtoNFA(NFATableT *t, MGnode *n)
{
	MGedge *e;
	MGedgelist *l;
	MGnode *next;
	int i;

	if (n!=NULL)
	{
		if (!n->visited)
		{
			n->visited = 1;
			if (n->edgelistout != NULL)
			{
				// for each edge in this node
				for (l=n->edgelistout; l!=NULL; l=l->next)
				{
					e = l->edge;
					// for the target of this edge
					next = e->target;
					for (i=0; i<256; i++)
						// if it's in the char set 
						if (InCharSet(i, (CharSetT *) e->label))
						{
							// add it to the list of states for this symbol from state n
							if (t->TTable[256*n->id + i] == NULL)
								t->TTable[256*n->id + i] = new intdynarray();
							t->TTable[256*n->id + i]->addunique(next->id);
							// if this state is accepting
							if (next->accepting)
								// add it to the list of final states
								t->finalStates.addunique(next->id);
						}
				}
				// for each edge in this node
				for (l=n->edgelistout; l!=NULL; l=l->next)
				{
					// do the destination
					recursiveMGtoNFA(t, l->edge->target);
				}
			}
		}
	}
}

void eliminateNFAdups(NFATableT *t)
{
	int i, j, l;
	int same;
	int null;
	int finalcount;
	int fs;
	int f;

	for (i=0; i<t->numstates; i++)
		for (j=0; j<t->numstates; j++)
			if (i != j)
			{
				same = 1;
				null = 1;
				for (l=0; l<256; l++)
				{	// if they are not equal as addresses (might both be null)
					if (t->TTable[256*i + l] != NULL && t->TTable[256*j + l] != NULL)
					{	// if their lists are not equal
						null = 0;
						if (!t->TTable[256*i + l]->sameset(t->TTable[256*j + l]))
						{
							same = 0;
							l = 256;
						}
					}
					else if (t->TTable[256*i + l] != t->TTable[256*j + l])
					{
						same = 0;
						null = 0;
					}
				}
				if (same && !null)
					NFAreplace(t, i, j);
			}
}

// replace j with i
void NFAreplace(NFATableT *t, int i, int j)
{
	int k;
	int l;
	int c;
	intdynarray *tt;
	
	for (k=0; k<t->numstates; k++)
	{
		for (l=0; l<256; l++)
		{
			tt = NFATTablead(t, k, l);
			if (tt != NULL)
				if (tt->nelem())
					for (c = 0; c < tt->nelem(); c++)
						if (tt->get(c) == j)
							tt->set(c, i);
		}
	}
	for (k=0; k<t->finalStates.nelem(); k++)
		if(t->finalStates.get(k) == j)
			t->finalStates.set(k, i);
	NFAremove(t, j);
}

// replace j with i
void NFAremove(NFATableT *t, int r)
{
	int k;
	int l;
	int c;
	int i;
	intdynarray *tt;
	
	for (k=0; k<t->numstates; k++)
	{
		for (l=0; l<256; l++)
		{
			tt = NFATTablead(t, k, l);
			if (tt != NULL)
				if (tt->nelem())
					for (c = 0; c < tt->nelem(); c++)
					{
						i = tt->get(c);
						if (i > r)
							tt->set(c, i-1);
					}
		}
	}
	for (k=r; k<t->numstates-1; k++)
		for (l=0; l<256; l++)
			t->TTable[k*256+l] = t->TTable[(k+1)*256+l];
	for (k=0; k<t->finalStates.nelem(); k++)
		if((i = t->finalStates.get(k)) > r)
			t->finalStates.set(k, i-1);
				
	t->numstates--;
}

void printNFA(NFATableT *t, char *regexpstr)
{
	int i, j, k;
	intdynarray *tt;
	
	printf("NFA\n");
	printf("%s\n", regexpstr);
	printf("number of states = %d\n", t->numstates);
	printf("Start State = %d\n", t->start);
	printf("Final States = {");
	for (i = 0; i < t->finalStates.nelem() - 1; i++)
	{
		printf("%d, ", t->finalStates.get(i));
	}
	printf("%d}\n", t->finalStates.get(i));
	
	for (i = 0; i < t->numstates; i++)
	{
		printf("State q%d:\n", i);  // index is the name of the state
		for (j = 0; j < 256; j++) 
		{
			tt = NFATTablead(t, i, j);
			if (tt != NULL)
			{
				if (tt->nelem())
				{
					if (isprint(j))
						printf("\tInput '%c': {", (char)j);
					else
						printf("\tInput #%d: {", j);
					for (k = 0; k < tt->nelem() - 1; k++) 
					{
						printf("%d, ", tt->get(k));
					}
					printf("%d}\n", tt->get(k));
				}
			}
		}
	}
	printf("End NFA\n");
}

int DFAnextState(DFATableT *d, int state, int symbol)
{
	return (d->nextstate[state*256 + symbol]);
}

void convertMGtoNFA(void)
{
	unvisitallnodes();
	initNFATable(nMGnodes, start);
	recursiveMGtoNFA(&NFATable, start);
}

void convertNFAtoDFA(DFATableT *d, NFATableT *n)
{
	int row;
	int nextrow;
	int label;
	intdynarray *dt, *dt2, *nt;
	int r;
	int state;
	int j, k;
	intdynarray **TTable;
	
	TTable = (intdynarray **) malloc(256*sizeof(intdynarray *));
	for (j=0; j<256; j++)
		TTable[j] = NULL;

	initDFATable(d);
	
	// add first row = empty
	DFAaddrow(d);
//	printf("added row, numrows = %d\n", d->numrows);
	
	// add next row = NFA start state
	DFAaddrow(d);
//	printf("added row, numrows = %d\n", d->numrows);
	d->NFAstates[1]->addunique(n->start);
	d->start = 1;
	// if the start state is also a final state,
	if (n->finalStates.find(n->start))
		// add it to the final states of the DFA
		d->finalStates.addunique(1);
	
	// for each existing row in our DFA
	for (row=1; row<d->numrows; row++)
	{
		for (j=0; j<256; j++)
		{
			if (TTable[j] != NULL)
			{
				delete TTable[j];
				TTable[j] = NULL;
			}
		}
//		printf("processing DFA row %d\n", row);
		// for each row, set the next states as the union of all next states 
		// 		from each NFA state in the state name for each symbol
		// for each NFA state in this row's name (name must be complete at this point)
		for (k = 0; k < d->NFAstates[row]->nelem(); k++)
		{
//			printf("this row name has %d elements.  ", d->NFAstates[row]->nelem());
//			d->NFAstates[row]->print(" = ", " ");
			state = d->NFAstates[row]->get(k);
//			printf("row name %d = %d\n", k, state);
			if (state != -1)
			{
				// for each possible label
				for (label=0; label<256; label++)
				{
					// find the next states from the NFA
					if (n->TTable[256*state + label] == NULL)
						n->TTable[256*state + label] = new intdynarray();
					nt = n->TTable[256*state + label];
					if (nt->nelem())
					{	
//						printf("for label %c add ", label);
						if (TTable[label] == NULL)
							TTable[label] = new intdynarray();
						// add each next state for this label to that for the DFA
						for (j=0; j<nt->nelem(); j++)
						{
//							printf("%d=%d ", j, nt->get(j));
							TTable[label]->addunique(nt->get(j));
						}
					}
				}
			}
		}
		// now we should have next state sets complete for this row as sets of NFA states
		// next for each label we look for that state in the set of names
		// if we don't find it we add it as a new row
		// Once we have it, we set the nextrow = target row number, and set
		// nextstate[row][symbol] = nextrow
		for (label=0; label<256; label++)
		{
			if (TTable[label] != NULL)
			{
				dt = TTable[label];
				if  (dt->nelem())
				{
//					printf("label %c: ", label);
//					dt->print("", "\n");
					// look for this set in the set of NFAstate names of the DFA
					for (j=0; j<d->numrows; j++)
					{
						if (*dt == *(d->NFAstates[j]))
						{
//							printf("found a match at row %d ", j);
//							d->NFAstates[row]->print("", "\n");
							nextrow = j;
							goto setnextstate;
						}
					}
					// we did not find this state set in the set of rows
					nextrow = DFAaddrow(d);
					// just to make sure...(should have been created empty in addrow)
//					d->NFAstates[nextrow]->clear();
					for (k=0; k<dt->nelem(); k++)
						d->NFAstates[nextrow]->addunique(dt->get(k));
//					printf("did not find a match, added row %d ", nextrow);
//					d->NFAstates[nextrow]->print("", "\n");
setnextstate:
					d->nextstate[row*256 + label] = nextrow;
//					printf("set nextstate = %d\n", d->nextstate[row*256 + label]);
					// now see if any of these NFA states appear in NFA's final states
					for (j=0; j<d->NFAstates[nextrow]->nelem(); j++)
						if (n->finalStates.find(d->NFAstates[nextrow]->get(j)))
							d->finalStates.addunique(nextrow);
				}
			}
		}
	}
}

void printDFA(DFATableT *d, char *regexpstr)
{
	int i, j, k;
	intdynarray *tt;
	
	printf("DFA\n");
	printf("%s\n", regexpstr);
	printf("number of states = %d\n", d->numrows);
	printf("Start State = %d\n", d->start);
	printf("Final States = {");
	for (i = 0; i < d->finalStates.nelem() - 1; i++) 
	{
		printf("%d, ", d->finalStates.get(i));
	}
	printf("%d}\n", d->finalStates.get(i));
	
	for (i = 0; i < d->numrows; i++) 
	{
		printf("State q%d:\n", i);  // row number is the name of the state
		for (j = 0; j < 256; j++) 
		{
			if (DFAnextState(d, i, j) != -1)
			{
				if (isprint(j))
					printf("\tInput '%c': {", (char)j);
				else
					printf("\tInput #%d: {", j);
				printf("%d}\n", DFAnextState(d, i, j));
			}
		}
	}
	printf("End DFA\n");
}

