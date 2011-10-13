/*
** cmd_list.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Fri Oct  7 10:56:22 2011 Jonathan Machado
** Last update Wed Oct 12 17:38:40 2011 Jonathan Machado
*/

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include "flowstat.h"

extern int		serv_socket;
extern int		clnt_socket;
extern struct global_info	info;

static void		exit_cmd(char **param);
static void	       	kill_cmd(char **param);
static void		stat_cmd(char **param);
static void		flux_cmd(char **param);
static void		ip_cmd(char **param);

struct cmd_info		list[] =
  {
    {"exit", 0, &exit_cmd},
    {"kill", 0, &kill_cmd},
    {"flux", 1, &flux_cmd},
    {"ip", 0, &ip_cmd},
    {"stat", 1, &stat_cmd},
    {NULL,0,NULL}
  };

static void		exit_cmd(char **param)
{
  (void)param;
  send(clnt_socket, "Bye, bye ...\n", strlen("Bye, bye ...\n"), 0);
  close(clnt_socket);
}

static char		*get_line_info(flux *flx)
{
  int			len;
  char			*ret;
  char			*date;

  len = 0;
  ret = xmalloc(512 * sizeof(*ret));
  switch (flx->protocol)
    {
    case IPPROTO_UDP:
      len += snprintf(&ret[len], 515 - len, "UDP|");
      len += snprintf(&ret[len], 515 - len, "%u|", flx->protocol_data.udp.port);
      break;
    case IPPROTO_ICMP:
      len += snprintf(&ret[len], 515 - len, "ICMP|");
      switch (flx->protocol_data.icmp.type)
	{
	case ICMP_ECHO:
	  len += snprintf(&ret[len], 515 - len, "Echo|");
	  break;
	case ICMP_ECHOREPLY:
	  len += snprintf(&ret[len], 515 - len, "Echo reply|");
	  break;
	case ICMP_DEST_UNREACH:
	  len += snprintf(&ret[len], 515 - len, "Destination Unreachable|");
	  break;
	default:
	  len += snprintf(&ret[len], 515 - len, "Other|");
	}
      break;
    case IPPROTO_TCP:
      len += snprintf(&ret[len], 515 - len, "TCP|");
      len += snprintf(&ret[len], 515 - len, "%u|", flx->protocol_data.tcp.port);
      break;
    default:
      len += snprintf(&ret[len], 515 - len, "OTHER|");
    }
    date = ctime(&(flx->first_packet));
    date[24] = 0;
    len += snprintf(&ret[len], 515 - len, "%s|", date);
    date = ctime(&(flx->last_packet));
    date[24] = 0;
    len += snprintf(&ret[len], 515 - len, "%s|", date);
    if (info.options.advanced) {
      len += snprintf(&ret[len], 515 - len, "%u:", (int)((flx->last_packet - flx->first_packet) / 60));
      len += snprintf(&ret[len], 515 - len, "%u|", (int)((flx->last_packet - flx->first_packet) % 60));
      if (flx->input_packet != 0)
	{
	  len += snprintf(&ret[len], 515 - len, "%u|", flx->input_packet);
	  len += snprintf(&ret[len], 515 - len, "%u|", flx->input_data);
	  len += snprintf(&ret[len], 515 - len, "%.2f|", (float)flx->input_data / (flx->last_packet - flx->first_packet + 1));
	  len += snprintf(&ret[len], 515 - len, "%.2f|", (float)flx->input_data / flx->input_packet);
	}
      if (flx->output_packet != 0)
	{
	  len += snprintf(&ret[len], 515 - len, "%u|", flx->output_packet);
	  len += snprintf(&ret[len], 515 - len, "%u|", flx->output_data);
	  len += snprintf(&ret[len], 515 - len, "%.2f|", (float)flx->output_data / (flx->last_packet - flx->first_packet + 1));
	  len += snprintf(&ret[len], 515 - len, "%.2f|", (float)flx->output_data / flx->output_packet);
	}
    }
  snprintf(&ret[len], 515 - len, "\n");
  return (ret);
}

static void	       	kill_cmd(char **param)
{
  free_tab(param);
  send(clnt_socket, "Deamon is shuting Down ...\n",
       strlen("Deamon is shuting Down ...\n"), 0);
  pthread_cancel(info.threads[0]);
  pthread_cancel(info.threads[1]);
  close(clnt_socket);
  close(serv_socket);
  free_at_interupt();
}

static void		stat_cmd(char **param)
{
  int			ip;
  char			buff[BUFFER_SIZE];
  connection		*cnt = NULL;

  inet_pton(AF_INET, param[1], &ip);
  cnt = ip_already_listed(ntohl(ip));
  if (cnt == NULL) {
    send(clnt_socket, "No connection listed for this ip\n",
	 strlen("No connection listed for this ip\n"), 0);
  }
  else {
    if (info.options.dns)
      snprintf(buff, BUFFER_SIZE, "hostname: %s\nok:%u ko:%u udp:%u icmp:%u other:%u\n",
	       cnt->hostname, cnt->stat.ok, cnt->stat.ko, cnt->stat.udp, cnt->stat.icmp,
	       cnt->stat.other);
    else
      snprintf(buff, BUFFER_SIZE, "ok:%u ko:%u udp:%u icmp:%u other:%u\n",
	       cnt->stat.ok, cnt->stat.ko, cnt->stat.udp, cnt->stat.icmp,
	       cnt->stat.other);
    send(clnt_socket, buff, strlen(buff), 0);
    if (cnt->stat.last_ko != NULL) {
      snprintf(buff, BUFFER_SIZE, "last ko:\nstart: %slast: %sinput: %u output: %u\n",
	       ctime(&cnt->stat.last_ko->first_packet), ctime(&cnt->stat.last_ko->last_packet),
	       cnt->stat.last_ko->input_data, cnt->stat.last_ko->output_data);
      send(clnt_socket, buff, strlen(buff), 0);
    }
  }
}

static void		flux_cmd(char **param)
{
  int			ip = 0;
  char			*buff = NULL;
  connection		*cnt = NULL;
  flux			*flx = NULL;

  inet_pton(AF_INET, param[1], &ip);
  cnt = ip_already_listed(ntohl(ip));
  if (cnt == NULL) {
    send(clnt_socket, "No connection listed for this ip\n",
	 strlen("No connection listed for this ip\n"), 0);
  } else {
    pthread_mutex_lock(&cnt->lock);
    send(clnt_socket, "Flux connected:\n", strlen("Flux connected:\n"), 0);
    for (flx = cnt->head; flx != NULL; flx = flx->next) {
      buff = get_line_info(flx);
      send(clnt_socket, buff, strlen(buff), 0);
      free(buff);
    }
    pthread_mutex_unlock(&cnt->lock);
  }
}

static void		ip_cmd(char **param)
{
  int			net_ip;
  char			ip[INET_ADDRSTRLEN];
  char			buff[BUFFER_SIZE];
  connection		*cur = NULL;
  (void)param;

  for (cur = info.head; cur != NULL; cur = cur->next) {
    net_ip = htonl(cur->ip);
    inet_ntop(AF_INET, &net_ip, ip, INET_ADDRSTRLEN);
    snprintf(buff, BUFFER_SIZE, "%s\n", ip);
    send(clnt_socket, buff, strlen(buff), 0);
  }
}
