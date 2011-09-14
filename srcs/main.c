/*
** main.c for flowstat in /home/jonathan/flowstat/srcs
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Fri Sep  2 12:00:57 2011 Jonathan Machado
** Last update Wed Sep 14 15:03:31 2011 Jonathan Machado
*/

#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <signal.h>
#include "flowstat.h"
#include "cson_amalgamation_core.h"

struct global_info	info;

int			get_local_ip(void)
{
  int			ret;
  struct ifaddrs	*myaddrs, *ifa;
  struct sockaddr_in	*socket;

  if(getifaddrs(&myaddrs) != 0)
    {
      flowstat_perror(NULL);
      exit(EXIT_FAILURE);
    }
  for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
    {
      if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET)
        {
          socket = (struct sockaddr_in *)ifa->ifa_addr;
          if (strcmp(ifa->ifa_name, "eth0") == 0)
            ret = socket->sin_addr.s_addr;
	}
    }
  freeifaddrs(myaddrs);
  return (ntohl(ret));
}

void			create_new_json(void)
{
  cson_value		*fluxV;

  info.rootV = cson_value_new_object();
  info.root = cson_value_get_object(info.rootV);
  fluxV = cson_value_new_array();
  info.flux = cson_value_get_array(fluxV);
  cson_object_set(info.root, "Log start", cson_value_new_integer(time(NULL)));
  cson_object_set(info.root, "flux", fluxV);
}

void			read_and_analyze(struct ipulog_handle *h)
{
  int			len;
  ulog_packet_msg_t	*ulog_packet;

  while ((len = ipulog_read(h, info.buffer, BUFFER_SIZE, 1)))
    while ((ulog_packet = ipulog_get_packet(h, info.buffer, len)))
      packet_handler(ulog_packet);
}

void save_json_in_file(void)
{
  FILE *file;
  cson_output_opt opt;

  file = fopen("./flowstat.log", "w");
  cson_object_set(info.root, "Last save", cson_value_new_integer(time(NULL)));
  opt = cson_output_opt_empty;
  opt.indentation = 1;
  cson_output_FILE(info.rootV, file, &opt);
  fclose(file);
}

void			free_at_interupt(int signum)
{
  save_json_in_file();
  free(info.buffer);
  ipulog_destroy_handle(info.connection);
  cson_value_free(info.rootV);
  exit(0);
}

void			init(void)
{
  openlog("flowstat : ", LOG_PID, LOG_USER);
  info.connection = verified_ipulog_create_handle(ipulog_group2gmask(GROUP_NETLINK), 150000);
  info.local_ip = get_local_ip();
  info.buffer = xmalloc(BUFFER_SIZE * sizeof(*info.buffer));
  signal(SIGINT, &free_at_interupt);
}

int			main(void)
{
  init();
  read_and_analyze(info.connection);
  return (EXIT_SUCCESS);
}
