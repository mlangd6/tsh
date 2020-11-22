/* list.c - Double linked list structures and functions definitions */
#include <stdlib.h>
#include <stdbool.h> 

#include "list.h"

typedef struct cell cell;
struct cell
{
  cell *prev;
  cell *next;
  void *val;
};


struct list_t
{
  cell *first;
  cell *last;
};

static cell *create_cell(cell *p, cell *n, void *v)
{
  cell *c = malloc(sizeof(cell));

  if (c)
    {
      c->prev = p;
      c->next = n;
      c->val  = v;
    }
  
  return c;
}


/* Create an empty list */
list_t *create_list()
{
  list_t *l = malloc(sizeof(list_t));

  if (l)
    {
      l->first = NULL;
      l->last  = NULL;
    }

  return l;
}

/* free all elements used by LIST */
void free_list(list_t *list, bool full)
{
  // NULL
  if (!list)
    return;

  // liste vide
  if (!list->first)
    {
      free (list);
      return;
    }


  cell *current, *next;

  current = list->first;
  while (current)
    {
      next = current->next;
      
      if (full)
	free(current->val);
      
      free (current);
      current = next;
    }

  free(list);
}


void list_add_first(list_t *list, void *val)
{
  if (!list)
    return;

  if (!list->first)
    {
      list->first = create_cell(NULL, NULL, val);
      list->last = list->first;
    }
  else
    {
      list->first->prev = create_cell(NULL, list->first, val);
      list->first = list->first->prev;
    }
}


void list_add_last(list_t *list, void *val)
{
  if (!list)
    return;

  if (!list->last)
    {
      list->last = create_cell(NULL, NULL, val);
      list->first = list->last;
    }
  else
    {
      list->last->next = create_cell(list->last, NULL, val);
      list->last = list->last->next;
    }
}

void *list_first (list_t *list)
{
  if (!list || !list->first)
    return NULL;

  return list->first->val;
}

void *list_last (list_t *list)
{
  if (!list || !list->last)
    return NULL;

  return list->last->val;
}


int list_size (list_t *list)
{
  if (!list)
    return -1;
  
  int i = 0;
  for (cell *c = list->first; c; c = c->next)
    i++;

  return i;
}


int is_empty (list_t *list)
{
  if (!list)
    return -1;
  
  return list->first == NULL;
}


void list_map(list_t *list, void (*f)(void *))
{
  if(!list)
    return;
  
  for(cell *c = list->first; c; c = c->next)
    {
      f(c->val);
    }
}
