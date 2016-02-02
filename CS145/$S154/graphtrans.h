/* This module demonstrates converting a regular expression into a
   directed multigraph with edges labelled according to character set.  It
   also contains features to optimize by merging edges and removing
   epsilon productions 
*/

#ifndef GRAPH_TRANSLATION_H_
#define GRAPH_TRANSLATION_H_

#include "parser.h"

typedef struct MGedgelist {
	struct MGedge *edge;
	struct MGedgelist *previous;
	struct MGedgelist *next;
} MGedgelist;

typedef struct MGedge {
	CharSetT *label;
	struct MGnode *target;
	int id;
	int epsilon;
	int deletable;
} MGedge;

typedef struct MGnode {
	struct MGedgelist *edgelistout;
	struct MGedgelist *lastedgelistout;
	struct MGedgelist *edgelistin;
	struct MGedgelist *lastedgelistin;
	struct MGnode *previous;
	struct MGnode *next;
	int visited;
	int id;
	int accepting;
	int reachable;
} MGnode;
	
extern MGnode *start;
extern MGnode *final;
extern MGnode *nodelist;
extern int nMGnodes;

MGnode *newMGnode();
void freeMGnode(MGnode *node);
void freeMGedgelist(MGedgelist *list);
MGedge *newMGedge();
MGedge *newMGedge(CharSetT *label);
void labelMGedge(int c, MGedge* edge);
void removelabelMGedge(int c, MGedge* edge);
void rangelabelMGedge(int a, int b, MGedge* edge);
void removerangelabelMGedge(int a, int b, MGedge* edge);
void freeMGedge(MGedge *edge);
void addtonodelist(MGnode *node);
MGnode *findnode(MGnode *node);
void removenode(MGnode *node);
void connectMGnodes(MGnode *s, MGnode *f);
void connectMGnodes(MGnode *s, MGnode *f, CharSetT* charset);
void RecursivePTtoMG(RegExpressionT *t, MGnode *s, MGnode* f);
void addedgeout(MGnode *node, MGedge *edge);
void addedgein(MGnode *node, MGedge *edge);
void initMG();
void recursiveprintMG(MGnode *n);
void unvisitallnodes(void);
void printMG(void);
MGedgelist *newMGedgelist(MGedge *e);
void recursiveEpsilonRemoval(MGnode *n);
void removeEpsilons(void);
void removeedgein(MGnode *node, MGedge *edge);
void removeedgeout(MGnode *node, MGedge *edge);
int edgeinnode(MGnode *n, MGedge *e);
void recursiveReachNodes(MGnode *n);
void markReachable(void);
void markallnodesunreachable(void);
void printnodelist(void);
void freeUnreachableNodes(void);
void addedgelist(MGedge *edge);
void removeedgelist(MGedge *edge);
void deleteEdges(void);
void MGdeleteall(void);

#endif
