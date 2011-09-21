/*
** utils.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Tue Sep 20 11:22:53 2011 Jonathan Machado
** Last update Wed Sep 21 17:44:18 2011 Jonathan Machado
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <signal.h>
#include "flowstat.h"

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
          if (strcmp(ifa->ifa_name, "eth0") == 0) /* keep the ip of INTERFACE */
            ret = socket->sin_addr.s_addr;
	}
    }
  freeifaddrs(myaddrs);
  return (ntohl(ret));
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
  if (chdir("/tmp") < 0)   		/* change running directory */
    {
      flowstat_perror("chdir");
      exit(EXIT_FAILURE);
    }
  if ((fd = open("flowstat.lock", O_RDWR | O_CREAT, 0640)) < 0)
    {
      flowstat_perror("open");
      exit(EXIT_FAILURE);
    }
  if (lockf(fd, F_TLOCK, 0) < 0)
    return (0); 		/* another instance is already running */
  sprintf(str,"%d\n", getpid());
  if (write(fd, str, strlen(str)) < 0) /* record pid to lockfile */
    {
      flowstat_perror("write");
      exit(EXIT_FAILURE);
    }
  signal(SIGTSTP, SIG_IGN);	/* ignore tty signals */
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  return (1);
}
