/*
** log.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep 21 17:45:46 2011 Jonathan Machado
** Last update Wed Sep 21 17:48:38 2011 Jonathan Machado
*/

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include "flowstat.h"

extern struct global_info	info;

static char			*get_line_info(flux *flx, connection *cnt)
{
  int			len;
  char			*ret;
  char			*date;

  len = 0;
  ret = xmalloc(512 * sizeof(*ret));
  if (cnt->protocol == IPPROTO_TCP && cnt->protocol_data.tcp.stts == reseted)
    len += sprintf(&ret[len], "[RESETED]");
  len += sprintf(&ret[len], "%u.%u.%u.%u|", INTTOIP(flx->ip));
#ifdef DNS_ACTIVATE
  len += sprintf(&ret[len], "%s|", flx->hostname);
#endif	/* DNS_ACTIVATE */
  switch (cnt->protocol)
    {
    case IPPROTO_UDP:
      len += sprintf(&ret[len], "UDP|");
      len += sprintf(&ret[len], "%u|", cnt->protocol_data.udp.port);
      break;
    case IPPROTO_ICMP:
      len += sprintf(&ret[len], "ICMP|");
      switch (cnt->protocol_data.icmp.type)
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
      len += sprintf(&ret[len], "%u|", cnt->protocol_data.tcp.port);
      break;
    default:
      len += sprintf(&ret[len], "OTHER|");
    }
  date = ctime(&(cnt->first_packet));
  date[24] = 0;
  len += sprintf(&ret[len], "%s|", date);
  date = ctime(&(cnt->last_packet));
  date[24] = 0;
  len += sprintf(&ret[len], "%s|", date);
  len += sprintf(&ret[len], "%i:", (int)((cnt->last_packet - cnt->first_packet) / 60));
  len += sprintf(&ret[len], "%i|", (int)((cnt->last_packet - cnt->first_packet) % 60));
  if (cnt->input_packet != 0)
    {
      len += sprintf(&ret[len], "%i|", cnt->input_packet);
      len += sprintf(&ret[len], "%i|", cnt->input_data);
      len += sprintf(&ret[len], "%.2f|", (float)cnt->input_data / (cnt->last_packet - cnt->first_packet + 1));
      len += sprintf(&ret[len], "%.2f|", (float)cnt->input_data / cnt->input_packet);
    }
  if (cnt->output_packet != 0)
    {
      len += sprintf(&ret[len], "%i|", cnt->output_packet);
      len += sprintf(&ret[len], "%i|", cnt->output_data);
      len += sprintf(&ret[len], "%.2f|", (float)cnt->output_data / (cnt->last_packet - cnt->first_packet + 1));
      len += sprintf(&ret[len], "%.2f|", (float)cnt->output_data / cnt->output_packet);
    }
  return (ret);
}

void			flush_closed_connection(void)
{
  FILE			*file;
  char			*ret;
  flux			*current_flux;
  connection		*prev_connection;
  connection		*current_connection;

  if ((file = fopen("./log", "a")) == NULL)
    {
      flowstat_perror("fopen");
      exit(EXIT_FAILURE);
    }
  current_flux = info.head;
  while (current_flux != NULL)
    {
      prev_connection = NULL;
      current_connection = current_flux->head;
      while (current_connection != NULL)
	{
	  if (current_connection->protocol == IPPROTO_UDP || current_connection->protocol == IPPROTO_ICMP ||
	      (current_connection->protocol == IPPROTO_TCP &&
	       (current_connection->protocol_data.tcp.stts == closed ||
		current_connection->protocol_data.tcp.stts == reseted)))
	    {
	      ret = get_line_info(current_flux, current_connection);
	      fprintf(file, "%s\n", ret);
	      free(ret);
	      delete_connection(current_flux, prev_connection, current_connection);
	      current_connection = prev_connection;
	    }
	  prev_connection = current_connection;
	  if (current_connection == NULL)
	    current_connection = current_flux->head;
	  else
	    current_connection = current_connection->next;
	}
      current_flux = current_flux->next;
    }
  fclose(file);
}
