#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include "list.h"

typedef list stack;


/* Create an empty stack */
stack *stack_create ();

/* free STACK and all its elements.
   if FULL is true then free is used on data too */
void stack_free (stack *stack, bool full);



/* Get the size of STACK.
   Return -1 if STACK is NULL */
int stack_size (stack *stack);

/* Check if STACK is empty.
   Return -1 if STACK is NULL */
int stack_is_empty (stack *stack);



/* Push VAL onto the top of STACK */
void stack_push (stack *stack, void *val);



/* Remove the value at the top STACK.
   Returns NULL if STACK is empty otherwise the value at the top. */
void *stack_pop (stack *stack);

/* Get the element at the top of STACK
   Returns NULL if STACK is empty otherwise the value at the top. */
void *stack_peek (stack *stack);

#endif
