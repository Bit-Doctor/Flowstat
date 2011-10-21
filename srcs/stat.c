/*
** log.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep 21 17:45:46 2011 Jonathan Machado
** Last update Fri Oct 21 12:23:40 2011 Jonathan Machado
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
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
  if (size_flux_list(stat->history_head) > info.options.history_size)
    pop_and_push_flux(&stat->history_head, &stat->history_tail, flx);
  else
    add_flux_to_end_list(&stat->history_head, &stat->history_tail, flx);
  switch (flx->protocol)
    {
    case IPPROTO_TCP:
      if (flx->protocol_data.tcp.stts == closed) {
	stat->ok++;
      } else {
	++stat->ko;
	free(stat->last_ko);
	stat->last_ko = xmalloc(sizeof(flux));
	stat->last_ko = memcpy(stat->last_ko, flx, sizeof(flux));
      }
      break;
    case IPPROTO_ICMP:
      if (flx->protocol_data.icmp.type == ICMP_DEST_UNREACH) {
	++stat->ko;
	free(stat->last_ko);
	stat->last_ko = xmalloc(sizeof(flux));
	stat->last_ko = memcpy(stat->last_ko, flx, sizeof(flux));
      }
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
    if (pthread_mutex_trylock(&cur_connection->lock) == 0) {
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
      pthread_mutex_unlock(&cur_connection->lock);
    }
  }
}

void			*flush_and_calc(void *ptr)
{
  time_t		cur;
  time_t	       	prev;

  (void) ptr;
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL))
    {
      flowstat_perror("pthread_setcanceltype");
      exit(EXIT_FAILURE);
    }
  for (prev = time(NULL), cur = time(NULL);;cur = time(NULL)) {
    if (cur - prev > 60) {
      flush_closed_flux();
      prev = cur;
    } else {
      sleep(60 - (cur - prev));
    }
  }
  return (NULL);
}

