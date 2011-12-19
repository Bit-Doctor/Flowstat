/*
** utils.c for flowstat in /home/jonathan/Projets/test
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Tue Nov 22 15:08:33 2011 Jonathan Machado
** Last update Mon Dec 19 11:23:28 2011 Jonathan Machado
*/

#include <netdb.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "flowstat.h"

void	free_peer_t(peer_t *peer)
{
  free(peer->interface);
  free(peer->hostname);
  delete_list(peer->connections, &free);
  delete_list(peer->stat.history, &free);
  free(peer);
}

int	compare_connection(connection_t *cnt1, connection_t *cnt2)
{
  int	ret;

  ret = 1;
  if (cnt1->port == cnt2->port)
    ret = 0;
  return (ret);
}

int	compare_history(history_t *hist, int *port)
{
  int	ret;

  ret = 1;
  if (*port == hist->port)
    ret = 0;
  return (ret);
}

void   	flowstat_perror(char *str)
{
  if (str && !errno)
  syslog(LOG_ERR, "%s\n", str);
  else if (str && errno)
    syslog(LOG_ERR, "%s: %m\n", str);
  else if (errno)
    syslog(LOG_ERR, "%m\n");
}

void   	free_tab(char **tab)
{
  int	i;

  for (i = 0; tab[i] != NULL; i++) {
    free(tab[i]);
    tab[i] = NULL;
  }
  free(tab);
  tab = NULL;
}

char	*get_hostname(u_int32_t ip)
{
  struct sockaddr_in    socket;
  char                  *dns;

  dns = malloc(1024 * sizeof(*dns));
  memset(dns, 0, 1024 * sizeof(*dns));
  memset(&socket, 0, sizeof(socket));
  socket.sin_family = AF_INET;
  socket.sin_addr.s_addr = ip;
  socket.sin_port = htons(80);
  getnameinfo((const struct sockaddr *)&socket, sizeof(socket),
	      dns, 1024, NULL, 0, 0);
  return (dns);
}

void	free_message_t(message_queue_t *msg)
{
 struct {
    u_int8_t	fin;
    u_int8_t	ack;
    u_int8_t	rst;
    u_int32_t	idx;
    peer_t	*peer;
  }			*del;

  if (msg->type == ADD_PACKET) {
    del = msg->data;
    free_peer_t(del->peer);
    free(msg->data);
    free(msg);
  }
}

int    	demonize(void)
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
