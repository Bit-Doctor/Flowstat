/*
** list_utils.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep 21 09:41:42 2011 Jonathan Machado
** Last update Wed Sep 21 10:52:03 2011 Jonathan Machado
*/

#include "flowstat.h"

void			delete_connection(flux *current_flux, connection *prev, connection *delete)
{
  if (delete != NULL)
    {
      if (prev == NULL)
	current_flux->head = delete->next;
      else
	prev->next = delete->next;
      if (delete->next == NULL)
	current_flux->tail = prev;
      current_flux->number_connections--;
      free(delete);
    }
}

void			free_connection_list(connection *head)
{
  if (head != NULL)
    {
      if (head->next != NULL)
	free_connection_list(head->next);
      free(head);
    }
}

void			free_flux_list(flux *head)
{
  if (head != NULL)
    {
      if (head->next != NULL)
	free_flux_list(head->next);
#ifdef DNS_ACTIVATE
      free(head->hostname);
#endif	/* DNS_ACTIVATE */
      free_connection_list(head->head);
      free(head);
    }
}
