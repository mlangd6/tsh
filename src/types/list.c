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


struct list
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


list *list_create()
{
  list *l = malloc(sizeof(list));

  if (l)
    {
      l->first = NULL;
      l->last  = NULL;
    }

  return l;
}

void list_free(list *list, bool full)
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



int list_size (list *list)
{
  if (!list)
    return -1;

  int i = 0;
  for (cell *c = list->first; c; c = c->next)
    i++;

  return i;
}


int list_is_empty (list *list)
{
  if (!list)
    return -1;

  return list->first == NULL;
}



void list_insert_first(list *list, void *val)
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

void list_insert_last(list *list, void *val)
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


void *list_remove_first (list *list)
{
  if (list_is_empty(list))
    return NULL;

  void *ret = list->first->val;
  cell *next = list->first->next;

  if (next)
    {
      free(list->first);
      next->prev  = NULL;
      list->first = next;
    }
  else
    {
      free(list->first);
      list->first = NULL;
      list->last  = NULL;
    }


  return ret;
}


void *list_remove_last (list *list)
{
  if (list_is_empty(list))
    return NULL;

  void *ret = list->last->val;
  cell *prev = list->last->prev;

  if (prev)
    {
      free(list->last);
      prev->next  = NULL;
      list->last  = prev;
    }
  else
    {
      free(list->last);
      list->first = NULL;
      list->last  = NULL;
    }

  return ret;
}



void *list_first (list *list)
{
  if (!list || !list->first)
    return NULL;

  return list->first->val;
}


void *list_last (list *list)
{
  if (!list || !list->last)
    return NULL;

  return list->last->val;
}



void list_iter(list *list, void (*f)(void *))
{
  if(!list)
    return;

  for(cell *c = list->first; c; c = c->next)
    {
      f(c->val);
    }
}

bool list_for_all(list *list, bool (*predicate)(void *))
{
  if (!list)
    return false;
  for (cell *c = list -> first; c; c = c -> next)
  {
    if (!predicate(c -> val))
    {
      return false;
    }
  }
  return true;
}

void list_free_full(list *list, void (*free_func)(void *))
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

    free_func(current->val);

    free (current);
    current = next;
  }

  free(list);

}
