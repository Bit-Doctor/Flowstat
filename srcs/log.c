/*
** log.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep 21 17:45:46 2011 Jonathan Machado
** Last update Mon Sep 26 11:43:00 2011 Jonathan Machado
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include "flowstat.h"

extern struct global_info	info;

static char			*get_line_info(connection *cnt, flux *flx)
{
  int			len;
  char			*ret;
  char			*date;

  len = 0;
  ret = xmalloc(258 * sizeof(*ret));
  if (flx->protocol == IPPROTO_TCP && flx->protocol_data.tcp.stts == reseted)
    len += sprintf(&ret[len], "[RESETED]");
  len += sprintf(&ret[len], "%u.%u.%u.%u|", INTTOIP(cnt->ip));
  if (info.options.dns)
    len += sprintf(&ret[len], "%s|", cnt->hostname);
  switch (flx->protocol)
    {
    case IPPROTO_UDP:
      len += sprintf(&ret[len], "UDP|");
      len += sprintf(&ret[len], "%u|", flx->protocol_data.udp.port);
      break;
    case IPPROTO_ICMP:
      len += sprintf(&ret[len], "ICMP|");
      switch (flx->protocol_data.icmp.type)
	{
	case ICMP_ECHO:
	  len += sprintf(&ret[len], "Echo|");
	  break;
	case ICMP_ECHOREPLY:
	  len += sprintf(&ret[len], "Echo reply|");
	  break;
	case ICMP_DEST_UNREACH:
	  len += sprintf(&ret[len], "Destination Unreachable|");
	  break;
	default:
	  len += sprintf(&ret[len], "Other|");
	}
      break;
    case IPPROTO_TCP:
      len += sprintf(&ret[len], "TCP|");
      len += sprintf(&ret[len], "%u|", flx->protocol_data.tcp.port);
      break;
    default:
      len += sprintf(&ret[len], "OTHER|");
    }
  date = ctime(&(flx->first_packet));
  date[24] = 0;
  len += sprintf(&ret[len], "%s|", date);
  date = ctime(&(flx->last_packet));
  date[24] = 0;
  len += sprintf(&ret[len], "%s|", date);
  if (info.options.advanced)
    {
      len += sprintf(&ret[len], "%i:", (int)((flx->last_packet - flx->first_packet) / 60));
      len += sprintf(&ret[len], "%i|", (int)((flx->last_packet - flx->first_packet) % 60));
      if (flx->input_packet != 0)
	{
	  len += sprintf(&ret[len], "%i|", flx->input_packet);
	  len += sprintf(&ret[len], "%i|", flx->input_data);
	  len += sprintf(&ret[len], "%.2f|", (float)flx->input_data / (flx->last_packet - flx->first_packet + 1));
	  len += sprintf(&ret[len], "%.2f|", (float)flx->input_data / flx->input_packet);
	}
      if (flx->output_packet != 0)
	{
	  len += sprintf(&ret[len], "%i|", flx->output_packet);
	  len += sprintf(&ret[len], "%i|", flx->output_data);
	  len += sprintf(&ret[len], "%.2f|", (float)flx->output_data / (flx->last_packet - flx->first_packet + 1));
	  len += sprintf(&ret[len], "%.2f|", (float)flx->output_data / flx->output_packet);
	}
    }
  return (ret);
}

void			flush_closed_flux(void)
{
  FILE			*file;
  char			*ret;
  connection   		*current_connection;
  flux			*prev_flux;
  flux			*current_flux;

  if ((file = fopen("./log", "a")) == NULL)
    {
      flowstat_perror("fopen");
      exit(EXIT_FAILURE);
    }
  current_connection = info.head;
  while (current_connection != NULL)
    {
      prev_flux = NULL;
      current_flux = current_connection->head;
      while (current_flux != NULL)
	{
	  if (current_flux->protocol == IPPROTO_UDP || current_flux->protocol == IPPROTO_ICMP ||
	      (current_flux->protocol == IPPROTO_TCP &&
	       (current_flux->protocol_data.tcp.stts == closed ||
		current_flux->protocol_data.tcp.stts == reseted)))
	    {
	      ret = get_line_info(current_connection, current_flux);
	      fprintf(file, "%s\n", ret);
	      free(ret);
	      delete_flux(current_connection, prev_flux, current_flux);
	      current_flux = prev_flux;
	    }  prev_flux = current_flux;
	  if (current_flux == NULL)
	    current_flux = current_connection->head;
	  else
	    current_flux = current_flux->next;
	}
      current_connection = current_connection->next;
    }
  fclose(file);
}
