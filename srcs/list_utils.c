/*
** list_utils.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep 21 09:41:42 2011 Jonathan Machado
** Last update Thu Sep 29 11:38:31 2011 Jonathan Machado
*/

#include <stdlib.h>
#include "flowstat.h"

extern struct global_info	info;

flux			*extract_flux(connection *current_connection, flux *prev, flux *delete)
{
  if (delete != NULL) {
    if (prev == NULL)
      current_connection->head = delete->next;
    else
      prev->next = delete->next;
    if (delete->next == NULL)
      current_connection->tail = prev;
    current_connection->number_flow--;
    delete->next = NULL;
  }
  return (delete);
}

void			delete_flux(connection *current_connection, flux *prev, flux *delete)
{
  if (delete != NULL) {
    if (prev == NULL)
      current_connection->head = delete->next;
    else
      prev->next = delete->next;
    if (delete->next == NULL)
      current_connection->tail = prev;
    current_connection->number_flow--;
    free(delete);
    delete = NULL;
  }
}

void			free_flux_list(flux *head)
{
  if (head != NULL) {
    if (head->next != NULL)
      free_flux_list(head->next);
    free(head);
    head = NULL;
  }
}

void			free_connection_list(connection *head)
{
  if (head != NULL) {
    if (head->next != NULL)
      free_connection_list(head->next);
    if (info.options.dns)
      free(head->hostname);
    free_flux_list(head->head);
    free(head);
    head = NULL;
  }
}
