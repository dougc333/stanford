#include <stdio.h>

#include "stack.h"


void pop_print_all(const char *name, stack * s)
{
  printf("contents of %s are: \n", name);
  while(!stack__is_empty(s))
  {
    double d = stack__pop(s);
    printf("%f ", d);
  }
  printf("\n\n");
}


void foo(void)
{
  stack *r = stack__create(20);

  for(int i=60; i<80; ++i)
    stack__push(r, i);

  pop_print_all("r", r);

  stack__destroy(r);
}


int main()
{
  stack *s = stack__create(20);

  for(int i=0; i<10; ++i)
    stack__push(s, i);

  stack *t = stack__create_copy(s);

  pop_print_all("s", s);

  stack__destroy(s);

  foo();

  for(int i=10; i<20; ++i)
    stack__push(t, i);

  pop_print_all("t", t);

  stack__destroy(t);

  return 0;
}

