/*
** main.c for flowstat in /home/jonathan/flowstat/srcs
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Fri Sep  2 12:00:57 2011 Jonathan Machado
** Last update Fri Sep 16 10:47:30 2011 Jonathan Machado
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
      flowstat_perror("getifaddrs");
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

void			save_json_in_file(void)
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
  closelog();
  ipulog_destroy_handle(info.connection);
  cson_value_free(info.rootV);
  exit(EXIT_SUCCESS);
}

int			demonize()
{
  int			pid;
  int			fd;
  char			str[10];

  if(getppid() == 1)
    return (1);			/* already a daemon */
  if ((pid = fork()) < 0)
    {
      flowstat_perror("fork");
      exit(EXIT_FAILURE);
    }
  if (pid > 0)
    exit(EXIT_SUCCESS);		/* parent exits */
  setsid();			/* obtain a new process group */
  umask(027);			/* set newly created file permissions */
  chdir("/tmp");       		/* change running directory */
  if ((fd = open("flowstat.lock", O_RDWR | O_CREAT, 0640)) < 0)
    {
      flowstat_perror("open");
      exit(EXIT_FAILURE);
    }
  if (lockf(fd, F_TLOCK, 0) < 0)
    return (0); 		/* another instance is already running */
  sprintf(str,"%d\n", getpid());
  write(fd, str, strlen(str)); /* record pid to lockfile */
  signal(SIGTSTP, SIG_IGN);	/* ignore tty signals */
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  return (1);
}

void			init(void)
{
  create_new_json();
  info.connection = verified_ipulog_create_handle(ipulog_group2gmask(GROUP_NETLINK), 150000);
  info.local_ip = get_local_ip();
  info.buffer = xmalloc(BUFFER_SIZE * sizeof(*info.buffer));
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
