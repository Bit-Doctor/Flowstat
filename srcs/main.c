/*
** main.c for flowstat in /home/jonathan/flowstat/srcs
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Fri Sep  2 12:00:57 2011 Jonathan Machado
** Last update Fri Oct 21 15:49:22 2011 Jonathan Machado
*/

#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <pthread.h>
#include <unistd.h>
#include "flowstat.h"

struct global_info	info;

void			init(void)
{
  pthread_attr_t	attr;

  info.local_ip = get_local_ip();
  info.buffer = xmalloc(BUFFER_SIZE * sizeof(*info.buffer));
  info.connection = verified_ipulog_create_handle(ipulog_group2gmask(GROUP_NETLINK), 150000);
  info.number_connection = 0;
  info.head = NULL;
  info.tail = NULL;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  if (pthread_create(&info.threads[0], &attr, &read_and_analyze, info.connection)) {
    flowstat_perror("pthread");
    exit(EXIT_FAILURE);
  }
  if (pthread_create(&info.threads[1], &attr, &flush_and_calc, NULL)) {
    flowstat_perror("pthread");
    exit(EXIT_FAILURE);
  }
  if (pthread_create(&info.threads[2], &attr, &client_handler, NULL)) {
    flowstat_perror("pthread");
    exit(EXIT_FAILURE);
  }
  pthread_attr_destroy(&attr);
}

void			usage(char *str)
{
  printf("usage %s [options]\n"
	 "-h\t\tthis text you see right here\n"
	 "-d\t\tactivate DNS resolution\n"
	 "-l\t\tactivate detailed output\n"
	 "-L nb\tset a limit of different ip listed (5000 by default)\n"
	 "-H nb\tset the size of the closed connection's history (50 by default)\n"
	 ,str);
}

int			main(int ac, char **av)
{
  int			i;
  char			opt;
  void			*status = NULL;

  info.options.ip_limit = 5000;
  info.options.history_size = 50;
  while ((opt = getopt(ac, av, "dlhH:L:")) != EOF) {
    switch (opt) {
    case 'd':
      info.options.dns = 1;
      break;
    case 'l':
      info.options.advanced = 1;
      break;
    case 'L':
      info.options.ip_limit = atoi(optarg);
      break;
    case 'H':
      info.options.history_size = atoi(optarg);
      break;
    case 'h':
      usage(av[0]);
      exit(EXIT_SUCCESS);
    case '?':
      usage(av[0]);
      exit(EXIT_SUCCESS);
    default:
      usage(av[0]);
      exit(EXIT_SUCCESS);
    }
  }
  openlog("flowstat", LOG_PID, LOG_DAEMON);
  if (demonize() == 1) {
    init();
    for(i = 0; i < 3; i++) {
      if (pthread_join(info.threads[i], &status)) {
	flowstat_perror("pthread_join");
	exit(EXIT_FAILURE);
      }
    }
  } else {
    printf("[ERROR] : an instance is already running\n");
  }
  closelog();
  exit(EXIT_SUCCESS);
}
