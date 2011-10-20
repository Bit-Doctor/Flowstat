/*
** list_utils.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep 21 09:41:42 2011 Jonathan Machado
** Last update Thu Oct 20 11:13:46 2011 Jonathan Machado
*/

#include <stdlib.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>
#include "flowstat.h"

extern struct global_info	info;

/*
**	extract and return the node from the linked list
*/
flux			*extract_flux(connection *current_connection, flux *prev, flux *extract)
{
  if (extract != NULL) {
    if (prev == NULL)
      current_connection->head = extract->next;
    else
      prev->next = extract->next;
    if (extract->next == NULL)
      current_connection->tail = prev;
    current_connection->number_flow--;
    extract->next = NULL;
  }
  return (extract);
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

/*
**	free all nodes of flux
*/
void			free_flux_list(flux *head)
{
  if (head != NULL) {
    if (head->next != NULL)
      free_flux_list(head->next);
    free(head);
    head = NULL;
  }
}

/*
**	free all nodes of connection
*/
void			free_connection_list(connection *head)
{
  if (head != NULL) {
    if (head->next != NULL)
      free_connection_list(head->next);
    if (info.options.dns)
      free(head->hostname);
    free_flux_list(head->head);
    pthread_mutex_destroy(&head->lock);
    free(head);
    head = NULL;
  }
}
/*
**	return 1 if the flux and the info in pkt_info are the same
**	else return 0
*/
static int	       	is_the_same_flux(flux *current_flux, packet_info *pkt_info)
{
  int	       		ret;

  ret = 1;
  if (pkt_info->protocol != current_flux->protocol)
    ret = 0;
  switch (pkt_info->protocol) {
  case IPPROTO_ICMP:
    if (pkt_info->type != current_flux->protocol_data.icmp.type)
      ret = 0;
    break;
  case IPPROTO_TCP:
    if (current_flux->protocol_data.tcp.stts == closed || current_flux->protocol_data.tcp.stts == reseted)
      ret = 0;
    else if (pkt_info->port != current_flux->protocol_data.tcp.port)
      ret = 0;
    break;
  case IPPROTO_UDP:
    if (pkt_info->port !=  current_flux->protocol_data.udp.port)
      ret = 0;
    break;
  }
  return (ret);
}

/*
**	 Return a pointer to the listed flux if is_the_same_flux return 1
**	else return NULL if it'a a new flux
*/
flux    		*flux_already_listed(connection *current_connection, packet_info *pkt_info)
{
  flux		       	*current = NULL;

  current = current_connection->head;
  while (current) {
    if (is_the_same_flux(current, pkt_info))
      return (current);
    current = current->next;
  }
  return (NULL);
}

/*
** Same thing that flux_already_listed but for connection
*/
connection	*ip_already_listed(u_int32_t ip)
{
  connection           	*current = NULL;

  current = info.head;
  while (current != NULL) {
    if (ip == current->ip)
      return (current);
    current = current->next;
  }
  return (NULL);
}
