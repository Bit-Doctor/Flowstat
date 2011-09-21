/*
** main.c for flowstat in /home/jonathan/flowstat/srcs
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Fri Sep  2 12:00:57 2011 Jonathan Machado
** Last update Wed Sep 21 17:48:43 2011 Jonathan Machado
*/

#include <time.h>
#include <syslog.h>
#include <signal.h>
#include "flowstat.h"

struct global_info	info;

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
	if (time(NULL) - prev > 60 * 5)
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
