/*
** main.c for flowstat in /home/jonathan/flowstat/srcs
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Fri Sep  2 12:00:57 2011 Jonathan Machado
** Last update Wed Sep 21 15:16:59 2011 Jonathan Machado
*/

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <time.h>

#include <syslog.h>
#include <signal.h>
#include "flowstat.h"

struct global_info	info;

char			*get_line_info(flux *flx, connection *cnt)
{
  int			len;
  char			*ret;
  char			*date;

  ret = xmalloc(512 * sizeof(*ret));
  len = sprintf(ret, "%u.%u.%u.%u|", INTTOIP(flx->ip));
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
  len += sprintf(&ret[len], "%i:", ((cnt->last_packet - cnt->first_packet) / 60));
  len += sprintf(&ret[len], "%i|", ((cnt->last_packet - cnt->first_packet) % 60));
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

void			free_at_interupt(int signum)
{
  free(info.buffer);
  ipulog_destroy_handle(info.connection);
  free_flux_list(info.head);
  closelog();
  exit(EXIT_SUCCESS);
}

void			read_and_analyze(struct ipulog_handle *h)
{
  time_t		prev;
  int			len;
  ulog_packet_msg_t	*ulog_packet;

  prev = time(NULL);
  while ((len = ipulog_read(h, info.buffer, BUFFER_SIZE, 1)))
    while ((ulog_packet = ipulog_get_packet(h, info.buffer, len)))
      {
	packet_handler(ulog_packet);
	if (time(NULL) - prev > 6)
	  {
	    flush_closed_connection();
	    prev = time(NULL);
	  }
      }
}

void			init(void)
{
  info.local_ip = get_local_ip();
  info.buffer = xmalloc(BUFFER_SIZE * sizeof(*info.buffer));
  info.connection = verified_ipulog_create_handle(ipulog_group2gmask(GROUP_NETLINK), 150000);
  info.number_flux = 0;
  info.head = NULL;
  info.tail = NULL;
  signal(SIGINT, &free_at_interupt); /* a suprimer */
}

int			main(void)
{
  openlog("flowstat", LOG_PID, LOG_DAEMON);
  /* if (demonize()) */
  /*   { */
  init();
  read_and_analyze(info.connection);
  /*   } */
  /* else */
    /* { */
    /*   printf("cleint\n"); */
    /* } */
  closelog();
  return (EXIT_SUCCESS);
}
