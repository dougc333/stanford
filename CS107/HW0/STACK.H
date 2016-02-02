#ifndef __STACK_H__
#define __STACK_H__

/* 
  stack.h

  The stack module implements a simple stack of doubles

  (c) 2000 Hiroshi Ishii <hishii@cs.stanford.edu>
*/


struct stack
{
  double * data;    // array of elements
  int max_elems;    // maximum number of allowed elements
  int top;          // index of top of stack plus one
};


stack * stack__create(int max_elems);     // create an empty stack
stack * stack__create_copy(stack * s);    // create deep copy of given stack
void stack__destroy(stack * s);           // destroy stack

bool stack__is_empty(stack *s);       // true if stack empty, false otherwise
bool stack__is_full(stack *s);        // true if stack full, false otherwise

double stack__pop(stack * s);             // pop top of stack 
void stack__push(stack * s, double data); // push data onto stack


#endif // ! __STACK_H__

