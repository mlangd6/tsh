/* stack.c - Stack data structure functions definitons */

#include <stdbool.h>
#include "stack.h"


/* Create an empty stack */
stack *stack_create ()
{
  return list_create();
}

/* free STACK and all its elements */
void stack_free (stack *stack, bool full)
{
  list_free(stack, full);
}



/* Get the size of STACK */
int stack_size (stack *stack)
{
  return list_size (stack);
}



/* Check if STACK is empty */
int stack_is_empty (stack *stack)
{
  return list_is_empty (stack);
}



/* Push VAL onto the top of STACK */
void stack_push (stack *stack, void *val)
{
  list_insert_last (stack, val);
}



/* Remove the value at the top STACK */
void *stack_pop (stack *stack)
{
  return list_remove_last (stack);
}

/* Get the element at the top of STACK */
void *stack_peek (stack *stack)
{
  return list_last (stack);
}
