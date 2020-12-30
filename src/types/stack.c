#include <stdbool.h>

#include "stack.h"

stack *stack_create ()
{
  return list_create();
}

void stack_free (stack *stack, bool full)
{
  list_free(stack, full);
}

int stack_size (stack *stack)
{
  return list_size (stack);
}

int stack_is_empty (stack *stack)
{
  return list_is_empty (stack);
}

void stack_push (stack *stack, void *val)
{
  list_insert_last (stack, val);
}

void *stack_pop (stack *stack)
{
  return list_remove_last (stack);
}

void *stack_peek (stack *stack)
{
  return list_last (stack);
}
