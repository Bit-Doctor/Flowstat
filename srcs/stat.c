/*
** log.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep 21 17:45:46 2011 Jonathan Machado
** Last update Thu Sep 29 17:23:48 2011 Jonathan Machado
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include "flowstat.h"

extern struct global_info	info;

static void	       	update_stat(connection *cnt, flux *flx)
{
  struct flux_stat     	*stat = NULL;

  stat = &cnt->stat;
  switch (flx->protocol)
    {
    case IPPROTO_TCP:
      if (flx->protocol_data.tcp.stts == closed) {
	stat->ok++;
      } else {
	stat->ko++;
	free(stat->last_ko);
	stat->last_ko = xmalloc(sizeof(*flx));
	stat->last_ko = memcpy(flx, stat->last_ko, sizeof(flx));
      }
      break;
    case IPPROTO_ICMP:
      stat->icmp++;
      break;
    case IPPROTO_UDP:
      stat->udp++;
      break;
    default:
      stat->other++;
      break;
    }
}

static void		flush_closed_flux(void)
{
  connection   		*cur_connection = NULL;
  flux			*prev_flux = NULL;
  flux			*cur_flux = NULL;
  flux			*extract = NULL;

  for (cur_connection = info.head; cur_connection != NULL; cur_connection = cur_connection->next) {
    for (prev_flux = NULL, cur_flux = cur_connection->head; cur_flux != NULL;) {
      if (!(cur_flux->protocol == IPPROTO_TCP &&
	    (cur_flux->protocol_data.tcp.stts != reseted &&
	     cur_flux->protocol_data.tcp.stts != closed))) {
	extract = extract_flux(cur_connection, prev_flux, cur_flux);
	update_stat(cur_connection, extract);
	free(extract);
	extract = NULL;
	cur_flux = prev_flux;
      }
      prev_flux = cur_flux;
      if (cur_flux == NULL)
	cur_flux = cur_connection->head;
      else
	cur_flux = cur_flux->next;
    }
  }
}

void			*flush_and_calc(void *ptr)
{
  time_t		cur;
  time_t	       	prev;

  (void) ptr;
  for (prev = time(NULL), cur = time(NULL);;cur = time(NULL)) {
    if (cur - prev < 60) {
      flush_closed_flux();
      prev = cur;
    } else {
      sleep(60 - (cur - prev));
    }
  }
  /* calc stat */
  return (NULL);
}
