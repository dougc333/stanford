//
// dynarray.h
// dynamic array class
// later convert to templates
//

#define INITALLOC 1

#include <stdio.h>

class intdynarray {
	private:
		int *a;
		int nalloc;
		int nelems;
		
	public:
		intdynarray();
		~intdynarray();
		void add(int val);
		void clear();
		int find(int val);
		int addunique(int val);
		void set(int i, int val);
		int get(int i);
		int nelem() { return nelems;}
		void check(int n);
		void remove(int n);
		void removeval(int n);
		int operator==(intdynarray& rhs);
		int operator!=(intdynarray& rhs);
		intdynarray &operator=(intdynarray& from);
		int contains(intdynarray *d);
		int sameset(intdynarray *d);

		void print(char *head, char *tail);
};
