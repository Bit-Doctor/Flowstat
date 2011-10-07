/*
** cmd_list.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Fri Oct  7 10:56:22 2011 Jonathan Machado
** Last update Fri Oct  7 11:40:41 2011 Jonathan Machado
*/

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "flowstat.h"

extern int		serv_socket;
extern int		clnt_socket;
extern struct global_info	info;
static void	       	kill_cmd(char **param);
static void		stat_cmd(char **param);
static void		list_cmd(char **param);
struct cmd_info		list[] =
  {
    {"kill", 0, &kill_cmd},
    {"list", 1, &list_cmd},
    {"stat", 1, &stat_cmd},
    {NULL,0,NULL}
  };

static void	       	kill_cmd(char **param)
{
  free_tab(param);		/* test */
  send(clnt_socket, "Deamon is shuting Down ...\n",
       strlen("Deamon is shuting Down ...\n"), 0);
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
  cnt = ip_already_listed(ip);
  if (cnt == NULL) {
    send(clnt_socket, "No connection listed for this ip\n",
	 strlen("No connection listed for this ip\n"), 0);
  }
  else {
    if (info.options.dns)
      snprintf(buff, BUFFER_SIZE, "%u.%u.%u.%u %s ok:%u ko:%u last ko:%s\n",
	       INTTOIP(cnt->ip), cnt->hostname, cnt->stat.ok, cnt->stat.ko,
	       ctime(&cnt->stat.last_ko->last_packet));
    else
      snprintf(buff, BUFFER_SIZE, "%u.%u.%u.%u ok:%u ko:%u last ko:%s\n",
	       INTTOIP(cnt->ip), cnt->stat.ok, cnt->stat.ko,
	       ctime(&cnt->stat.last_ko->last_packet));
    send(clnt_socket, buff, strlen(buff), 0);
  }
  free(param);
}

static void		list_cmd(char **param)
{
  int			ip = 0;
  connection		*cnt = NULL;

  inet_pton(AF_INET, param[1], &ip);
  cnt = ip_already_listed(ip);
  if (cnt == NULL) {
    send(clnt_socket, "No connection listed for this ip\n",
	 strlen("No connection listed for this ip\n"), 0);
  } else {
    pthread_mutex_lock(&cnt->lock);
    /* sprintf les connection active */
    pthread_mutex_unlock(&cnt->lock);
  }
  free(param);
}
