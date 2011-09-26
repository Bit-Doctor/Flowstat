/*
** main.c for flowstat in /home/jonathan/flowstat/srcs
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Fri Sep  2 12:00:57 2011 Jonathan Machado
** Last update Mon Sep 26 11:31:19 2011 Jonathan Machado
*/

#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "flowstat.h"

struct global_info	info;

void			*listen_client(void * titi)
{
  puts("hello");
  return (titi);
}

void			free_at_interupt(int signum)
{
  (void)signum;
  free(info.buffer);
  ipulog_destroy_handle(info.connection);
  free_connection_list(info.head);
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
	    flush_closed_flux();
	    prev = time(NULL);
	  }
      }
}

void			init(void)
{
  pthread_t		thread;

  info.local_ip = get_local_ip();
  info.buffer = xmalloc(BUFFER_SIZE * sizeof(*info.buffer));
  info.connection = verified_ipulog_create_handle(ipulog_group2gmask(GROUP_NETLINK), 150000);
  info.number_connection = 0;
  info.head = NULL;
  info.tail = NULL;
  signal(SIGINT, &free_at_interupt);
  if (pthread_create(&thread, NULL, &listen_client, NULL))
    {
      flowstat_perror("pthread");
      exit(EXIT_FAILURE);
    }
}

void			usage(char *str)
{
  printf("usage %s [options]\n"
	 "-h\t\tthis text you see right here\n"
	 "-D\t\tdemonize\n"
	 "-d\t\tactivate DNS resolution\n"
	 "-l\t\tactivate detailed output\n"
	 ,str);
}

int			main(int ac, char **av)
{
  char			opt;
  int			ret;

  ret = -1;
  while ((opt = getopt(ac, av, "dDlh")) != EOF)
    {
     switch (opt)
       {
       case 'd':
  	 info.options.dns = 1;
  	 break;
       case 'D':
  	 ret = demonize();
  	 break;
       case 'l':
  	 info.options.advanced = 1;
  	 break;
       case 'h':
  	 usage(av[0]);
  	 exit(EXIT_SUCCESS);
       case '?':
  	 usage(av[0]);
  	 exit(EXIT_SUCCESS);
       }
    }
  openlog("flowstat", LOG_PID, LOG_DAEMON);
  if (ret == -1 || ret == 1)
    {
      init();
      read_and_analyze(info.connection);
    }
  else
    {
      printf("[ERROR] : an instance is already running\n");
    }
  closelog();
  return (EXIT_SUCCESS);
}
