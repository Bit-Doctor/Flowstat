/*
** link.c for lib_list in /home/jonathan/Projets/utils/lib_linked_list
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Mon Oct 24 10:18:44 2011 Jonathan Machado
** Last update Thu Dec  8 15:39:06 2011 Jonathan Machado
*/

#include <stdlib.h>
#include <string.h>
#include "libdatac_link.h"

Link		*new_link(void)
{
  Link		*new = NULL;

  new = malloc(sizeof(*new));
  memset(new, 0, sizeof(*new));
  return (new);
}

Link		*new_link_by_copy(Link *link)
{
  Link		*new = NULL;

  if (link != NULL) {
    new = new_link();
    if (link->ptr != NULL) {
      new->ptr = malloc(sizeof(*(link->ptr)));
      new->ptr = memcpy(new->ptr, link->ptr, sizeof(*(link->ptr)));
    }
  }
  return (new);
}

Link		*new_link_by_param(void *ptr, size_t s)
{
  Link		*new = NULL;

  new = new_link();
  if (ptr != NULL) {
    new->ptr = malloc(s);
    new->ptr = memcpy(new->ptr, ptr, s);
  }
  return (new);
}

void		delete_link(Link *link, void (*f)(void*))
{
  if (link != NULL) {
    f(link->ptr);
    link->ptr = NULL;
    free(link);
    link = NULL;
  }
}

