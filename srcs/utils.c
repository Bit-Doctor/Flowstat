/*
** utils.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Tue Sep 20 11:22:53 2011 Jonathan Machado
** Last update Fri Oct 14 16:57:14 2011 Jonathan Machado
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include "flowstat.h"

extern struct global_info	info;

int			get_local_ip(void)
{
  int			ret;
  struct ifaddrs	*myaddrs, *ifa;
  struct sockaddr_in	*socket;

  ret = 0;
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
          if (strcmp(ifa->ifa_name, INTERFACE) == 0) /* keep the ip of INTERFACE */
            ret = socket->sin_addr.s_addr;
	}
    }
  freeifaddrs(myaddrs);
  return (ntohl(ret));
}

int			demonize(void)
{
  int			pid;
  int			fd;
  char			str[10];

  if(getppid() == 1)
    return (1);				/* already a daemon */
  if ((pid = fork()) < 0) {
    flowstat_perror("fork");
    exit(EXIT_FAILURE);
  }
  /*
  ** exit the parent
  ** then obtain a new process group
  ** and set newly created file permissions
  ** then change running directory
  */
  if (pid > 0)
    exit(EXIT_SUCCESS);
  setsid();
  umask(027);
  if (chdir("/tmp") < 0) {
    flowstat_perror("chdir");
    exit(EXIT_FAILURE);
  }
  if ((fd = open("flowstat.lock", O_RDWR | O_CREAT, 0640)) < 0) {
    flowstat_perror("open");
    exit(EXIT_FAILURE);
  }
  /* try to lockfile if it's fail another instance is already running
  ** else record pid to lockfile
  ** and ignore tty signals
  */
  if (lockf(fd, F_TLOCK, 0) < 0)
    return (0);
  sprintf(str,"%d\n", getpid());
  if (write(fd, str, strlen(str)) < 0) {
    flowstat_perror("write");
    exit(EXIT_FAILURE);
  }
  signal(SIGTSTP, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  return (1);
}

void			free_at_interupt(void)
{
  free(info.buffer);
  info.buffer = NULL;
  ipulog_destroy_handle(info.connection);
  info.connection = NULL;
  free_connection_list(info.head);
  info.head = NULL;
  closelog();
  pthread_exit((void*)EXIT_SUCCESS);
}

void			free_tab(char **tab)
{
  int			i;

  for (i = 0; tab[i] != NULL; i++) {
    free(tab[i]);
    tab[i] = NULL;
  }
  free(tab);
  tab = NULL;
}
