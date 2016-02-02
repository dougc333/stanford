//
// dynarray.c
// dynamic array class
// later convert to templates
//

#include "dynarray.h"
#include <stdlib.h>
#include <stdio.h>

intdynarray::intdynarray(void)
{
	int i;
	
	nalloc = INITALLOC;
	a = (int *) malloc(nalloc*sizeof(int));
	if (!a)
		perror("malloc failed in intdynarray constructor\n");
	for (i=0; i<nalloc; i++)
		a[i] = -1;
	nelems = 0;
}

intdynarray::~intdynarray(void)
{
	free(a);
}

void intdynarray::check(int n)
{
	int i, start;
	
	if (n >= nalloc)
	{
		start = nalloc;
		while (n >= nalloc)
			nalloc *=2;
		a = (int *) realloc(a, nalloc*sizeof(int));
		if (!a)
			perror("realloc failed in intdynarray");
		for (i=start; i<nalloc; i++)
			a[i] = -1;
	}
}

void intdynarray::add(int val)
{
	check(nelems);
	a[nelems] = val;
	nelems++;
//	printf("added %d to %d, nelem = %d\n", val, this, nelems);
}

int intdynarray::operator==(intdynarray& rhs)
{
	int i;
	
	if (this->nelems != rhs.nelems)
		return (0);
	for (i=0; i<this->nelems; i++)
	{
		if (this->get(i) != rhs.get(i))
			return (0);
	}
	return (1);
}

int intdynarray::operator!=(intdynarray& rhs)
{
	int i;
	
	if (this->nelems != rhs.nelems)
		return (1);
	for (i=0; i<this->nelems; i++)
	{
		if (this->get(i) != rhs.get(i))
			return (1);
	}
	return (0);
}

intdynarray &intdynarray::operator=(intdynarray& from)
{
	int i;
	
	free(a);
	intdynarray();
	nelems = 0;
	for (i=0; i<from.nelems; i++)
		this->set(i, from.get(i));
	return (*this);
}

void intdynarray::clear()
{
	free(a);
	intdynarray();
	nelems = 0;
}

// remove the item indexed by n
void intdynarray::remove(int n)
{
	int i;
	
	if (n >= nelems)
	{
		printf("i out of range in intdynarray.remove: i=%d, nelems=%d\n", i,
nelems); 		return;
	}	
	for (i=n; i<nelems-1; i++)
		a[i] = a[i+1];
	nelems--;
}

// remove the first item found with value n
void intdynarray::removeval(int n)
{
	int i;
	
	for (i=0; i<nelems; i++)
		if (a[i] == n)
		{
			remove(i);
			break;
		}
}

// add to array only if this value does not already appear
// returns 0 if it was already set (this call did not do anything)
int intdynarray::addunique(int val)
{
	int found = find(val);
	
	if (!found)
		add(val);
	return (!found);
}

void intdynarray::set(int i, int val)
{
	check(i);
	a[i] = val;
	// make sure we know that we have at least i elements (some of which may be empty)
	if (i >= nelems)
		nelems = i+1;
}

int intdynarray::get(int i)
{
	if (i >= nelems)
		printf("i out of range in intdynarray.get: i=%d, nelems=%d\n", i, nelems);
	return (a[i]);
}

// this is tricky because we may have undefined (= -1) elements via the set function
int intdynarray::find(int val)
{
	int i;
	
	for (i=0; i<nelems; i++)
		if (a[i] == val)
			return 1;
	return 0;
}

int intdynarray::contains(intdynarray *d)
{
	int i;
	
	for (i=0; i<d->nelems; i++)
		if (!find(d->get(i)))
			return 0;
	return 1;
}

int intdynarray::sameset(intdynarray *d)
{
	if (contains(d) && d->contains(this))
		return 1;
	return 0;
}

void intdynarray::print(char *head, char *tail)
{
	int i;
	
	printf("%s: ", head);
	for (i=0; i<nelems; i++)
		printf("%d=%d ", i, get(i));
	printf("%s", tail);
}

