/*
** cmd.c for flowstat in /home/jonathan/Projets/test
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Mon Nov 28 10:33:47 2011 Jonathan Machado
** Last update Mon Dec 19 14:56:44 2011 Jonathan Machado
*/

#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "flowstat.h"

extern int		serv_socket;
extern int		clnt_socket;

static void		help_cmd(global_info *info, char **param);
static void		exit_cmd(global_info *info, char **param);
static void	       	kill_cmd(global_info *info, char **param);
static void		stat_cmd(global_info *info, char **param);
static void		connection_cmd(global_info *info, char **param);
static void		flux_cmd(global_info *info, char **param);
static void		ip_cmd(global_info *info, char **param);
static void		flush_cmd(global_info *info, char **param);

cmd_info_t		cmd_list[] =
  {
    {"help", 0, &help_cmd},
    {"exit", 0, &exit_cmd},
    {"kill", 0, &kill_cmd},
    {"connection", 2, &connection_cmd},
    {"flux", 2, &flux_cmd},
    {"ip", 0, &ip_cmd},
    {"stat", 2, &stat_cmd},
    {"flush",0, &flush_cmd},
    {NULL,0,NULL},
  };

static void		help_cmd(global_info *info, char **param)
{
  (void)param;
  (void)info;
  send(clnt_socket, "connection exit flush flux help ip stat kill\n",
       strlen("connection exit flush flux help ip stat kill\n"), 0);
}

static void		flush_cmd(global_info *info, char **param)
{
  message_queue_t	message;

  (void)param;
  message.type = RESET_HASH_TABLE;
  if (sem_init(&message.semaphore, 0, 0) == -1)
    flowstat_perror("sem_init");
  g_async_queue_push(info->packet_queue, &message);
  sem_wait(&message.semaphore);
}

static void		exit_cmd(global_info *info, char **param)
{
  (void)param;
  (void)info;
  send(clnt_socket, "Bye, bye ...\n", strlen("Bye, bye ...\n"), 0);
  close(clnt_socket);
}

/*
** change info->finish to 1 in order to finish the deamon
** and send a message in the queue to "unblock" the thread
*/
static void	       	kill_cmd(global_info *info, char **param)
{
  message_queue_t	message;
  (void)param;

  info->finish = 1;
  send(clnt_socket, "Deamon is shuting Down ...\n",
       strlen("Deamon is shuting Down ...\n"), 0);
  message.type = KILL;
  if (sem_init(&message.semaphore, 0, 0) == -1)
    flowstat_perror("sem_init");
  g_async_queue_push(info->packet_queue, &message);
  sem_wait(&message.semaphore);
}

static void		stat_cmd(global_info *info, char **param)
{
  char			buff[BUFFER_SIZE];
  u_int32_t		key;
  u_int32_t		saddr;
  u_int32_t		daddr;
  peer_stat_t		*stat = NULL;
  message_queue_t	message;

  inet_pton(AF_INET, param[1], &saddr);
  inet_pton(AF_INET, param[2], &daddr);
  key = saddr + daddr;
  message.type = GET_PEER_STAT;
  message.data = &key;
  if (sem_init(&message.semaphore, 0, 0) == -1)
    flowstat_perror("sem_init");
  g_async_queue_push(info->packet_queue, &message);
  sem_wait(&message.semaphore);
  stat = message.data;
  memset(buff, 0, BUFFER_SIZE * sizeof(*buff));
  if (stat != NULL) {
    snprintf(buff, BUFFER_SIZE, "tcp:%u udp:%u icmp:%u other:%u ko:%u\n",
	     stat->tcp, stat->udp, stat->icmp, stat->other, stat->ko);
    send(clnt_socket, buff, BUFFER_SIZE, 0);
  } else {
    send(clnt_socket, "unknow connection\n",
         strlen("unknow connection\n"), 0);
  }
}

static void		send_cnt_info(connection_t *cnt)
{
  char	buff[BUFFER_SIZE];

  memset(buff, 0, BUFFER_SIZE * sizeof(*buff));
  snprintf(buff, BUFFER_SIZE, "%u|%u|%u|%u|%u|%u|%u\n", cnt->port,
	   cnt->in_packet, cnt->in_data, cnt->out_packet, cnt->out_data,
	   (unsigned int)cnt->first_packet, (unsigned int)cnt->last_packet);
  send(clnt_socket, buff, strlen(buff), 0);
}

static void		connection_cmd(global_info *info, char **param)
{
  u_int32_t		key;
  u_int32_t		saddr;
  u_int32_t		daddr;
  List			*list = NULL;
  message_queue_t	message;

  inet_pton(AF_INET, param[1], &saddr);
  inet_pton(AF_INET, param[2], &daddr);
  key = saddr + daddr;
  message.type = GET_CONNECTIONS_LIST;
  message.data = &key;
  if (sem_init(&message.semaphore, 0, 0) == -1)
    flowstat_perror("sem_init");
  g_async_queue_push(info->packet_queue, &message);
  sem_wait(&message.semaphore);
  list = message.data;
  if (list != NULL) {
    if (list->size != 0) {
      send(clnt_socket, "port|ip_pkt|in_data|out_pkt|out_data|first_pkt|last_pkt\n",
	   strlen("port|ip_pkt|in_data|out_pkt|out_data|first_pkt|last_pkt\n"), 0);
      iterate(list, (void (*)(void *))&send_cnt_info);
    } else {
      send(clnt_socket, "no active connections\n",
	   strlen("no active connections\n"), 0);
    }
  } else {
    send(clnt_socket, "unknow connection\n",
	 strlen("unknow connection\n"), 0);
    }
}

static void		send_history(history_t *hist)
{
  char	buff[BUFFER_SIZE];

  memset(buff, 0, BUFFER_SIZE * sizeof(*buff));
  snprintf(buff, BUFFER_SIZE, "port : %u nb : %lu first : %u last : %u\n",
	   hist->port, hist->nb, (unsigned int)hist->first, (unsigned int)hist->last);
  send(clnt_socket, buff, strlen(buff), 0);
}

static void		flux_cmd(global_info *info, char **param)
{
  u_int32_t		key;
  u_int32_t		saddr;
  u_int32_t		daddr;
  List			*list = NULL;
  message_queue_t	message;

  inet_pton(AF_INET, param[1], &saddr);
  inet_pton(AF_INET, param[2], &daddr);
  key = saddr + daddr;
  message.type = GET_HISTORY;
  message.data = &key;
  if (sem_init(&message.semaphore, 0, 0) == -1)
    flowstat_perror("sem_init");
  g_async_queue_push(info->packet_queue, &message);
  sem_wait(&message.semaphore);
  list = message.data;
  if (list != NULL) {
    if (list->size != 0) {
      iterate(list, (void (*)(void *))&send_history);
    } else {
      send(clnt_socket, "no history\n", strlen("no history\n"), 0);
    }
  } else {
    send(clnt_socket, "unknow connection\n",
	 strlen("unknow connection\n"), 0);
    }
}

static void		ip_cmd(global_info *info, char **param)
{
  int			i;
  int			ret;
  char			buff[100];
  peer_t		**list = NULL;
  message_queue_t	message;
  struct in_addr	a;

  (void)param;
  i = 0;
  message.type = GET_IP_LIST;
  if (sem_init(&message.semaphore, 0, 0) == -1)
    flowstat_perror("sem_init");
  g_async_queue_push(info->packet_queue, &message);
  sem_wait(&message.semaphore);
  list = message.data;
  if (list[0] == NULL)
    send(clnt_socket, "No ip listed\n", strlen("No ip listed\n"), 0);
  while (list[i] != NULL) {
    ret = 0;
    a.s_addr = list[i]->ips.haddr;
    ret = snprintf(buff, 100, "%s", inet_ntoa(a));
    a.s_addr = list[i]->ips.paddr;
    snprintf(&buff[ret], 100 - ret," <-> %s | %s <-> %s\n", inet_ntoa(a),
	     list[i]->interface, list[i]->hostname);
    send(clnt_socket, buff, strlen(buff), 0);
    i++;
  }
  free(list);
}
