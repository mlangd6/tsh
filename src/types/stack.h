/**
 * @file stack.h
 * Stack data structure
 */

#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

#include "list.h"

/**
 * Stack
 * Note that the stack holds only address and not the pointed data
 */ 
typedef list stack;

/**
 * Create an empty stack 
 * @return a malloc'd empty stack
 */
stack *stack_create ();

/**
 * Free the memory allocated for a stack
 * @param stack a stack
 * @param full if `full` is `true` then free is used on data too 
 */
void stack_free (stack *stack, bool full);

/**
 * Get the size of a stack
 * @param stack a stack
 * @return the size of the stack or -1 if `stack` is `NULL`
 */
int stack_size (stack *stack);

/**
 * Check if a stack is empty
 * @param stack a stack
 * @return 
 * * 0 if `stack` is no empty
 * * 1 if `stack` is empty
 * * -1 if `stack` is `NULL`
 */
int stack_is_empty (stack *stack);

/**
 * Push value onto the top of a stack
 * @param stack a stack
 * @param val the address to insert
 */
void stack_push (stack *stack, void *val);

/**
 * Remove the top element from a stack
 * @param stack a stack
 * @return `NULL` if `stack` is empty; otherwise the address of the top element.
 */
void *stack_pop (stack *stack);

/**
 * Get the top element from a stack
 * @param stack a stack
 * @return `NULL` if `stack` is empty; otherwise the address of the top element.
 */
void *stack_peek (stack *stack);

#endif
