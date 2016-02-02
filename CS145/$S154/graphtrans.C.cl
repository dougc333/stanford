// graphtrans.c
// takes a parse tree and generates a directed multi graph with edges labled by
// character sets.

#include "graphtrans.h"
#include "stdlib.h"
#include "stdio.h"

#define DEBUG 0

MGnode *start;
MGnode *final;

MGnode *nodelist = NULL;
MGnode *lastnode = NULL;

MGedgelist *edgelist = NULL;
MGedgelist *lastedge = NULL;

int nMGnodes = 0;
int nMGedges = 0;

void addtonodelist(MGnode *node)
{
	if (nodelist == NULL)
		nodelist = node;
	else
	{
		node->previous = lastnode;	
		lastnode->next = node;	
	}
	lastnode = node;
}

void removenode(MGnode *node)
{
	if (node != NULL)
	{
		if (node->next != NULL && node->previous != NULL)
		{
			node->previous->next = node->next;
			node->next->previous = node->previous;
			freeMGnode(node);
		}
		else if (node->previous != NULL)// last in list
		{
			node->previous->next = NULL;
			lastnode = node->previous;
			freeMGnode(node);
		}
		else // first in list
		{
			if (node->next != NULL) //not the only entry
				nodelist = node->next;
			else
			{
				nodelist = NULL;
				lastnode = NULL;
			}
			freeMGnode(node);
		}
	}
}

MGnode *newMGnode()
{
	MGnode *node;
	
	node = (MGnode *) malloc(sizeof(MGnode));
	node->edgelistout = NULL;
	node->lastedgelistout = NULL;
	node->lastedgelistin = NULL;
	node->edgelistin = NULL;
	node->previous = NULL;
	node->next = NULL;
	node->visited = 0;
	node->id = nMGnodes;
	node->accepting = 0;
	node->reachable = 0;
	
	addtonodelist(node);
	
//	printf("created node #%d\n", nMGnodes);
	nMGnodes++;
	
	return node;
}

void freeMGnode(MGnode *node)
{
	MGedgelist *l;
	
	for (l=node->edgelistout; l!=NULL; l=l->next)
		freeMGedge(l->edge);
	for (l=node->edgelistin; l!=NULL; l=l->next)
		freeMGedge(l->edge);
		
	freeMGedgelist(node->edgelistout);
	freeMGedgelist(node->edgelistin);
	free(node);
	nMGnodes--;
}

MGedgelist *newMGedgelist(MGedge *e)
{
	MGedgelist *l = (MGedgelist *) malloc(sizeof(MGedgelist));
	l->edge = e;
	l->next = NULL;
	l->previous = NULL;
	
	return (l);
}

void freeMGedgelist(MGedgelist *list)
{
	if (list == NULL)
		return;
	if (list->next != NULL)
		freeMGedgelist(list->next);
	free(list);
}

void addedgelist(MGedge *edge)
{
	MGedgelist *l = newMGedgelist(edge);

	if (edgelist == NULL)
		edgelist = l;
	else
	{
		l->previous = lastedge;
		l->previous->next = l;
	}
	lastedge = l;
}

void removeedgelist(MGedge *edge)
{
	MGedgelist *l;
	
	if (edgelist == NULL)
		return;
	// first element in list
	else if (edgelist->edge == edge)
	{
		edgelist->next->previous = NULL;
		edgelist = edgelist->next;
	}
	// go hunting for this edge in the list
	else
	{
		for (l=edgelist; l!=NULL; l=l->next)
		{
			if (l->edge == edge)
			// found it...
			{
				l->next->previous = l->previous;
				l->previous->next = l->next;
			}			
		}
	}
}

void deleteEdges(void)
{
	MGedgelist *l, *temp;
	
	for (l=edgelist; l!=NULL; l=temp)
	{
		temp = l->next;
		if (l->edge->deletable)
		{
			removeedgelist(l->edge);
			freeMGedge(l->edge);
			freeMGedgelist(l);
		}
	}
}

void addedgeout(MGnode *node, MGedge *edge)
{
	MGedgelist *l = newMGedgelist(edge);

	if (node->edgelistout == NULL)
		node->edgelistout = l;
	else
	{
		l->previous = node->lastedgelistout;
		l->previous->next = l;
	}
	node->lastedgelistout = l;
}

void removeedgeout(MGnode *node, MGedge *edge)
{
	MGedgelist *l;
	
	if (node->edgelistout == NULL)
		return;
	// first element in list
	else if (node->edgelistout->edge == edge)
	{
		if (node->edgelistout->next != NULL)
		{
			node->edgelistout->next->previous = NULL;
			node->edgelistout = node->edgelistout->next;
		}
		else // only element in list
			node->edgelistout = NULL;
	}
	// go hunting for this edge in the list
	else
	{
		for (l=node->edgelistout; l!=NULL; l=l->next)
		{
			if (l->edge == edge)
			// found it...
			{
				l->next->previous = l->previous;
				l->previous->next = l->next;
			}			
		}
	}
	edge->deletable = 1;
}

void addedgein(MGnode *node, MGedge *edge)
{
	MGedgelist *l = newMGedgelist(edge);

	if (node->edgelistin == NULL)
		node->edgelistin = l;
	else
	{
		l->previous = node->lastedgelistin;
		node->lastedgelistin->next = l;
	}
	node->lastedgelistin = l;
}

void removeedgein(MGnode *node, MGedge *edge)
{
	MGedgelist *l;
	
	if (node->edgelistin == NULL)
		return;
	// first element in list
	else if (node->edgelistin->edge == edge)
	{
		if (node->edgelistin->next != NULL)
		{
			node->edgelistin = node->edgelistin->next;
			if (node->edgelistin->next != NULL)
				node->edgelistin->next->previous = NULL;
		}
		else	// no following element
			node->edgelistin = NULL;
	}
	// go hunting for this edge in the list
	else
	{
		for (l=node->edgelistin; l!=NULL; l=l->next)
		{
			if (l->edge == edge)
			// found it...
			{
				if (l->next != NULL)
				{
					l->next->previous = l->previous;
					l->previous->next = l->next;
				}
				else
					l->previous->next = NULL;					
			}			
		}
	}		
}

MGedge *newMGedge(CharSetT *label)
{
	MGedge *edge;
	
	edge = (MGedge *) malloc(sizeof(MGedge));
	edge->label = label;
	edge->target = NULL;
	edge->epsilon = 0;
	edge->id = nMGedges;
	edge->deletable = 0;
//	printf("created edge #%d\n", nMGedges);
	nMGedges++;

	return edge;
}

MGedge *newMGedge()
{
	MGedge *edge = newMGedge(CreateCharSet());
	edge->epsilon = 1;
	
	return edge;
}

void labelMGedge(int c, MGedge* edge)
{
	AddCharacter(c, edge->label);
}

void removelabelMGedge(int c, MGedge* edge)
{
	RemoveCharacter(c, edge->label);
}

void rangelabelMGedge(int a, int b, MGedge* edge)
{
	AddCharacterRange(a, b, edge->label);
}

void removerangelabelMGedge(int a, int b, MGedge* edge)
{
	RemoveCharacterRange(a, b, edge->label);
}

void freeMGedge(MGedge *edge)
{
	free(edge->label);
	free(edge);
}

void connectMGnodes(MGnode *s, MGnode *f)
{
	connectMGnodes(s, f, NULL);
}

void connectMGnodes(MGnode *s, MGnode *f, CharSetT* charset)
{
	MGedge* edge;
	
	if (charset == NULL)
		edge = newMGedge(); // create epslion edge
	else
		edge = newMGedge(charset); // create labled edge
	
	edge->target = f;
	addedgeout(s, edge);
	addedgein(f, edge);
}

void initMG(void)
{	
	start = newMGnode();
	final = newMGnode();
	final->accepting = 1;
}

void RecursivePTtoMG(RegExpressionT *t, MGnode *s, MGnode* f)
{
	CharSetT* label;
	MGnode *q, *p, *q1, *q2, *p1, *p2;
	
	switch (t->type)
	{
		case EMPTY:
#if DEBUG
			printf("empty\n");
#endif
			connectMGnodes(s, f); // empty character set
		break;
			
		case ATOM:
#if DEBUG
			printf("atom = %c\n", (char)t->value);
#endif
			label = CreateCharSet();
			AddCharacter(t->value, label); // accept this one character
			connectMGnodes(s, f, label);
		break;
			
		case ANYCHAR:
#if DEBUG
			printf("any char\n");
#endif
			label = CreateCharSet();
			InvertCharSet(label); // accept all characters
			connectMGnodes(s, f, label);
		break;

		case CHARSET:
#if DEBUG
			printf("charset ");
			PrintCharSet(t->charset);
#endif
			connectMGnodes(s, f, t->charset);
		break;
		
		case UNION:
#if DEBUG
			printf("union\n");
#endif
			q1 = newMGnode();
			q2 = newMGnode();
			p1 = newMGnode();
			p2 = newMGnode();
			connectMGnodes(s, q1); // with epsilon edge
			connectMGnodes(s, q2); // with epsilon edge
			connectMGnodes(p1, f); // with epsilon edge
			connectMGnodes(p2, f); // with epsilon edge			
			RecursivePTtoMG(t->term, q1, p1);
			RecursivePTtoMG(t->next, q2, p2);
		break;
		
		case CONCAT:
#if DEBUG
			printf("concat\n");
#endif
			q = newMGnode();
			p = newMGnode();
			connectMGnodes(q, p); // with epsilon edge
			RecursivePTtoMG(t->term, s, q);
			RecursivePTtoMG(t->next, p, f);
		break;
		
		case KLEENE:
#if DEBUG
			printf("kleene\n");
#endif
			p = newMGnode();
			q = newMGnode();
			connectMGnodes(s, q); // with epsilon edge
			connectMGnodes(p, f); // with epsilon edge
			connectMGnodes(s, f); // with epsilon edge			
			connectMGnodes(p, q); // with epsilon edge			
			RecursivePTtoMG(t->term, q, p);
		break;
		
		case POSCLOSE:
#if DEBUG
			printf("posclose\n");
#endif
			p = newMGnode();
			q = newMGnode();
			connectMGnodes(s, q); // with epsilon edge
			connectMGnodes(p, f); // with epsilon edge
			connectMGnodes(p, q); // with epsilon edge			
			RecursivePTtoMG(t->term, q, p);
		break;
		
		case OPTION:
#if DEBUG
			printf("option\n");
#endif
			p = newMGnode();
			q = newMGnode();
			connectMGnodes(s, q); // with epsilon edge
			connectMGnodes(p, f); // with epsilon edge
			connectMGnodes(s, f); // with epsilon edge			
			RecursivePTtoMG(t->term, q, p);
		break;
		
		default:
			printf("invalid expression type %d\n", t->type);
		break;
		
	}
}

void recursiveEpsilonRemoval(MGnode *n)
{
	MGedge *e, *e1;
	MGedgelist *l, *el;
	MGnode *next;
	
	if (n!=NULL)
	{
		n->visited = 1;
		if (n->edgelistout != NULL)
		{
			// for each edge in this node
			for (l=n->edgelistout; l!=NULL; l=l->next)
			{
				e = l->edge;
				// if this is an epsilon edge
				if (e->epsilon)
				{
					next = e->target;
					// if we've not done the next node...
					if (!next->visited && next != n)
					{
						// for each edge in next node...
						for (el=next->edgelistout; el!=NULL; el=el->next)
						{
							e1 = el->edge;
							// if it's not already in node n,
							if (!edgeinnode(n, e1))
							{
								// add it to node n;
								addedgeout(n, e1);
							}
						}
					}
					n->accepting = next->accepting;
					removeedgein(e->target, e);
					removeedgeout(n, e);
				}
			}
			
			// for each edge in this node
			for (l=n->edgelistout; l!=NULL; l=l->next)
				if (!l->edge->target->visited)
				// process the next node
					recursiveEpsilonRemoval(l->edge->target);
		}		
	}	
}

int edgeinnode(MGnode *n, MGedge *e)
{
	MGedge *e1;
	MGedgelist *l;
	MGnode *next;
	
	if (n!=NULL)
	{
		if (n->edgelistout != NULL)
			for (l=n->edgelistout; l!=NULL; l=l->next)
				if (l->edge == e)
					return 1;
	}
	return 0;
}

void removeEpsilons(void)
{
	unvisitallnodes();
	recursiveEpsilonRemoval(start);
}

void markReachable(void)
{
	unvisitallnodes();
	markallnodesunreachable();
	recursiveReachNodes(start);
}

void recursiveReachNodes(MGnode *n)
{
	MGedgelist *l;
	MGnode *next;
	
	if (n!=NULL)
	{
		n->visited = 1;
		n->reachable = 1;
		if (n->edgelistout != NULL)
			for (l=n->edgelistout; l!=NULL; l=l->next)
			{
				next = l->edge->target;
				if (!next->visited)
					recursiveReachNodes(next);
			}
	}	
}

void freeUnreachableNodes(void)
{
	MGnode *n, *temp;
	int count = 0;
	
	for (n=nodelist; n!=NULL; n=temp)
	{
		temp = n->next;
		if (!n->reachable)
		{	
			removenode(n);
		}
	}
	// renumber the nodes
	for (n=nodelist; n!=NULL; n=n->next)
	{
		n->id = count;
		count++;
	}
//	printf("count = %d\n", count);
}

void recursiveprintMG(MGnode *n)
{
	MGedge *e;
	MGedgelist *l;
	MGnode *next;
	
	if (n!=NULL)
	{
		n->visited = 1;
		if (n->edgelistout != NULL)
			for (l=n->edgelistout; l!=NULL; l=l->next)
			{
				e = l->edge;
				printf("edge %d from %d to %d", e->id, n->id, e->target->id);
				if (e->target->accepting)
					printf(" (accepting)");
				printf(" label ");
				if (e->epsilon)
					printf("epsilon\n");
				else
					PrintCharSet(e->label);
				next = e->target;
				if (!next->visited)
					recursiveprintMG(next);
			}
	}	
}

void unvisitallnodes(void)
{
	MGnode *n;
	
	for (n=nodelist; n!=NULL; n=n->next)
		n->visited = 0;
}

void markallnodesunreachable(void)
{
	MGnode *n;
	
	for (n=nodelist; n!=NULL; n=n->next)
		n->reachable = 0;
}

void printMG(void)
{
	unvisitallnodes();
	recursiveprintMG(start);
	printf("done with printMG\n");
}

void printnodelist(void)
{
	MGnode *n;
	
	for (n=nodelist; n!=NULL; n=n->next)
	{
		printf("node %d: ", n->id);
		if (n->reachable)
			printf("reachable ");
		else
			printf("unreachable ");
		printf("\n");
	}	
}

void MGdeleteall()
{
	MGedgelist *l;
	MGnode *n;
	
	for (l=edgelist; l!=NULL; l=l->next)
		l->edge->deletable = 1;
	deleteEdges();
	
	freeMGedgelist(edgelist);
	freeMGedgelist(lastedge);
	while (nodelist != NULL)
		removenode(nodelist);
	start = final = NULL;
}
