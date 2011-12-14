/*
** list.c for lib_list in /home/jonathan/Projets/utils/lib_linked_list
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Mon Oct 24 14:47:22 2011 Jonathan Machado
** Last update Mon Dec 12 14:35:06 2011 Jonathan Machado
*/

#include <stdlib.h>
#include <string.h>
#include "libdatac_list.h"

List	*new_list(void)
{
  List	*new = NULL;

  new = malloc(sizeof(*new));
  new = memset(new, 0, sizeof(*new));
  return (new);
}

List	*new_list_by_copy(List *list)
{
  List	*new = NULL;
  Link	*cur = NULL;

  new = malloc(sizeof(*new));
  new = memset(new, 0, sizeof(*new));
  for_each(cur, list) {
    push_end(new, new_link_by_copy(cur));
  }
  return (new);
}

Link	*pop_front(List *list)
{
  Link	*ret = NULL;

  if (list != NULL) {
    if (list->head != NULL) {
      ret = list->head;
      if (ret->next != NULL)
	ret->next->prev = ret->prev;
      if (ret->prev != NULL)
	ret->prev->next = ret->next;
      list->head = ret->next;
      if (ret->next == NULL || ret == ret->next) {
	list->head = NULL;
	list->tail = NULL;
      }
      ret->next = NULL;
      ret->prev = NULL;
      list->size -= 1;
    }
  }
  return (ret);
}

Link	*pop_end(List *list)
{
  Link	*ret = NULL;

  if (list != NULL) {
    if (list->tail != NULL) {
      ret = list->tail;
      if (ret->next != NULL)
	ret->next->prev = ret->prev;
      if (ret->prev != NULL)
	ret->prev->next = ret->next;
      list->tail = ret->prev;
      if (ret->prev == NULL || ret == ret->prev) {
	list->head = NULL;
	list->tail = NULL;
      }
      ret->next = NULL;
      ret->prev = NULL;
      list->size -= 1;
    }
  }
  return (ret);
}

Link	*pop_at(List *list, unsigned int pos)
{
  unsigned int	i;
  Link	*ret = NULL;

  if (list != NULL) {
    if (list->size > pos) {
      if (pos == 0) {
	ret = pop_front(list);
      } else if (pos == list->size - 1) {
	ret = pop_end(list);
      } else {
	for (i = 0, ret = list->head; i < pos;
	     i++, ret = ret->next);
	ret->next->prev = ret->prev;
	ret->prev->next = ret->next;
	ret->next = NULL;
	ret->prev = NULL;
	list->size -= 1;
      }
    }
  }
  return (ret);
}

void	push_front(List *list, Link *new)
{
  if (list != NULL) {
    if (new != NULL) {
      new->prev = NULL;
      new->next = list->head;
      if (list->head != NULL) {
	if (list->head->prev != NULL)
	  new->prev = list->head->prev;
	if (list->tail->next != NULL)
	  list->tail->next = new;
	list->head->prev = new;
      }
      if (list->tail == NULL)
	list->tail = new;
      list->head = new;
      list->size += 1;
    }
  }
}

void	push_end(List *list, Link *new)
{
  if (list != NULL) {
    if (new != NULL) {
      new->next = NULL;
      new->prev = list->tail;
      if (list->tail != NULL) {
	if (list->tail->next != NULL)
	  new->next = list->tail->next;
	if (list->head->prev != NULL)
	  list->head->prev = new;
	list->tail->next = new;
      }
      if (list->head == NULL)
	list->head = new;
      list->tail = new;
      list->size += 1;
    }
  }
}

Link	*get_link(List *list, unsigned int pos)
{
  unsigned int	i;
  Link	*ret = NULL;

  if (list != NULL) {
    if (list->size > pos) {
      if (pos == 0)
	ret = list->head;
      else if (pos == list->size - 1)
	ret = list->tail;
      else
	for (i = 0, ret = list->head; i < pos;
	     i++, ret = ret->next);
    }
  }
  return (ret);
}

int	is_in_list(List *list, void *ptr, int (*f)(void *, void *))
{
  Link	*cur = NULL;

  if (list != NULL) {
    for_each(cur, list) {
      if (f == NULL) {
	if (memcmp(cur->ptr, ptr, sizeof(*ptr)) == 0)
	  return (1);
      } else {
	if (f(cur->ptr, ptr) == 0)
	  return (1);
      }
    }
  }
  return (0);
}

Link	*lookup(List *list, void *ptr, int (*f)(void *, void *))
{
  Link	*cur = NULL;

  if (list != NULL) {
    for_each(cur, list) {
      if (f == NULL) {
	if (memcmp(cur->ptr, ptr, sizeof(*ptr)) == 0)
	  return (cur);
      } else {
	if (f(cur->ptr, ptr) == 0)
	  return (cur);
      }
    }
  }
  return (NULL);
}

Link	*lookup_and_pop(List *list, void *ptr, int (*f)(void *, void *))
{
  unsigned int	i;
  Link	*cur = NULL;

  i = 0;
  if (list != NULL) {
    for_each(cur, list) {
      if (f == NULL) {
	if (memcmp(cur->ptr, ptr, sizeof(*ptr)) == 0)
	  return (pop_at(list, i));
      } else {
	if (f(cur->ptr, ptr) == 0) {
	  return (pop_at(list, i)); }
      }
      i++;
    }
  }
  return (NULL);
}

List	*iterate(List *list, void (*f)(void *))
{
  Link	*cur = NULL;

  if (list != NULL) {
    for_each(cur, list) {
      f(cur->ptr);
    }
  }
  return (list);
}

List	*concat(List *list1, List *list2)
{
  Link	*cur = NULL;

  if (list1 != NULL && list2 != NULL) {
    for_each(cur, list2) {
      push_end(list1, cur);
    }
  }
  return (list1);
}

static void	delete_link_list(Link *link, void (*f)(void *))
{
  if (link != NULL) {
    if (link->next != NULL)
      delete_link_list(link->next, f);
    delete_link(link, f);
  }
}

void	delete_list(List *list, void (*f)(void *))
{
  if (list->head != NULL)
    list->head->prev = NULL;
  if (list->tail != NULL)
    list->tail->next = NULL;
  delete_link_list(list->head, f);
  free(list);
  list = NULL;
}
