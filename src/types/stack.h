#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include "list.h"

typedef list_t stack_t;


/* Create an empty stack */
stack_t *stack_create ();

/* free STACK and all its elements. 
   if FULL is true then free is used on data too */
void stack_free (stack_t *stack, bool full);



/* Get the size of STACK.
   Return -1 if STACK is NULL */
int stack_size (stack_t *stack);

/* Check if STACK is empty.
   Return -1 if STACK is NULL */
int stack_is_empty (stack_t *stack);



/* Push VAL onto the top of STACK */
void stack_push (stack_t *stack, void *val);



/* Remove the value at the top STACK.
   Returns NULL if STACK is empty otherwise the value at the top. */ 
void *stack_pop (stack_t *stack);

/* Get the element at the top of STACK
   Returns NULL if STACK is empty otherwise the value at the top. */ 
void *stack_peek (stack_t *stack);

#endif
