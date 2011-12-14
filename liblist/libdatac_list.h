/*
** list.h for lib_list in /home/jonathan/Projets/utils/lib_linked_list
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Mon Oct 24 10:06:47 2011 Jonathan Machado
** Last update Thu Dec  8 15:59:22 2011 Jonathan Machado
*/

#ifndef __LIST_H__
# define __LIST_H__
# include "libdatac_link.h"

typedef	struct List	List;
struct			List
{
  unsigned int	size;
  Link		*head;
  Link		*tail;
};

#define for_each(pos, list) \
  for (pos = list->head; pos != NULL; pos = pos->next)

List		*new_list(void);
List		*new_list_by_copy(List *list);

Link		*pop_front(List *list);
Link		*pop_end(List *list);
Link		*pop_at(List *list, unsigned int pos);
void		push_front(List *list, Link *new);
void		push_end(List *list, Link *new);
Link		*get_link(List *list, unsigned int pos);

int		is_in_list(List *list, void *ptr, int (*f)(void *, void *));
Link		*lookup(List *list, void *ptr, int (*f)(void *, void *));
Link		*lookup_and_pop(List *list, void *ptr, int (*f)(void *, void *));

List		*revert(List *list);
List		*iterate(List *list, void (*f)(void *));

void		delete_list(List *list, void (*f)(void *));

/*	TO DO
**
** unique:
** insert : isert a list at a given pos
** get_link_pos : return the pos of a link cmp *ptr
** doc
*/

#endif	/* __LIST_H__ */
