/*
** flowstat.c for flowstat in /home/jonathan/Projets/test
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Thu Nov 10 09:18:39 2011 Jonathan Machado
** Last update Mon Dec 19 14:45:23 2011 Jonathan Machado
*/

#include <unistd.h>
#include <syslog.h>
#include <sys/socket.h>
#include <libnetfilter_log/libnetfilter_log.h>
#include "flowstat.h"

static int channel;

void	init(global_info *info)
{
  errno = 0;
  info->finish = 0;
  g_thread_init(NULL); 		/* ~159 alloc unfreeable*/
  info->packet_queue = g_async_queue_new_full((GDestroyNotify)&free_message_t);
  if (!info->packet_queue) {
    flowstat_perror("g_async_queue_new");
    info->finish = 1;
    exit(EXIT_FAILURE);
  }
  /*
  ** this is not init since nfa is not copied in packet_handler.c
  **
  ** info->packet_handler = g_thread_pool_new((GFunc)&packet_handler,
  ** 					   info, 100, TRUE, NULL);
  ** if (!info->packet_handler) {
  **     flowstat_perror("g_thread_pool_new");
  **   info->finish = 1;
  **   exit(EXIT_FAILURE);
  ** }
  **
  */
  if (g_thread_create((GThreadFunc)&hashtable_handler, info, TRUE, NULL) == NULL) {
    flowstat_perror("g_thread_create");
    info->finish = 1;
    exit(EXIT_FAILURE);
  }
  if (g_thread_create((GThreadFunc)&client_handler, info, FALSE, NULL) == NULL) {
    flowstat_perror("g_thread_create");
    info->finish = 1;
    exit(EXIT_FAILURE);
  }
  info->handle = nflog_open();
  if (!info->handle) {
    flowstat_perror("nflog_open");
    info->finish = 1;
    exit(EXIT_FAILURE);
  }
  if (nflog_unbind_pf(info->handle, AF_INET) < 0) {
    flowstat_perror("nflog_unbind_pf");
    info->finish = 1;
    exit(EXIT_FAILURE);
  }
  if (nflog_bind_pf(info->handle, AF_INET) < 0) {
    flowstat_perror("nflog_bind_pf");
    info->finish = 1;
    exit(EXIT_FAILURE);
  }
  info->g_handle = nflog_bind_group(info->handle, channel);
  if (!info->g_handle) {
    flowstat_perror("nflog_bind_group");
    info->finish = 1;
    exit(EXIT_FAILURE);
  }
  if (nflog_set_mode(info->g_handle, NFULNL_COPY_PACKET, 0xffff) < 0) {
    flowstat_perror("nflog_set_mode");
    info->finish = 1;
    exit(EXIT_FAILURE);
  }
  info->netlink_fd = nflog_fd(info->handle);
  nflog_callback_register(info->g_handle, &callback, info);
}

void			usage(char *str)
{
  printf("usage %s [options]\n"
	 "-h\t\tthis text you see right here\n"
	 "-c nb\t\tchange the channel (0 by default)\n"
	 "-l nb\tset a limit of different ip listed (no limit by default) if the limit is reached the table is flushed\n"
	 "-H nb\tset the size of the closed connection's history (50 by default)\n"
	 ,str);
}

void	fill_option(int ac, char **av, global_info *info)
{
  int	opt;

  info->options.max_peer = 0;
  info->options.history_size = 50;
  while ((opt = getopt(ac, av, "hH:l:c:")) != EOF) {
    switch (opt) {
    case 'c':
      channel =  atoi(optarg);
      break;
    case 'l':
      info->options.max_peer = atoi(optarg);
      break;
    case 'H':
      info->options.history_size = atoi(optarg);
      break;
    case 'h':
      usage(av[0]);
      g_thread_exit(NULL);
      break;
    case '?':
      usage(av[0]);
      g_thread_exit(NULL);
      break;
    default:
      usage(av[0]);
      g_thread_exit(NULL);
      break;
    }
  }
}

int	main(int ac, char **av)
{
  global_info	info;

  fill_option(ac, av, &info);
  openlog("flowstat", LOG_PID, LOG_DAEMON);
  if (demonize()) {
    init(&info);
    read_and_analyze(&info);
  } else {
    printf("%s\n", "daemon already running");
  }
  closelog();
  g_thread_exit(NULL);
  return (0);
}
