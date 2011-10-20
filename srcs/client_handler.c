/*
** client_handler.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Thu Sep 29 11:53:30 2011 Jonathan Machado
** Last update Tue Oct 18 19:08:52 2011 Jonathan Machado
*/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "flowstat.h"

int				serv_socket;
int				clnt_socket;
extern struct cmd_info		list[];

/*
**	compare the first word of the array with the knowed cmd in list[]
**	and check if the required number of parameter match else send
**	an error to the client, if the cmd is correct then call f()
*/
static void	       	handle_cmd(int nb, char **param)
{
  int			i;

  if (param[0] != NULL) {
    for (i = 0; list[i].cmd != NULL; i++) {
      if (strcmp(param[0], list[i].cmd) == 0) {
	if (nb == list[i].nb_param)
	  list[i].f(param);
	else
	  send(clnt_socket, "Bad number of parameter\n",
	       strlen("Bad number of parameter\n"), 0);
	break;
      }
    }
    if (list[i].cmd == NULL)
      send(clnt_socket, "Unknow command\n",
	   strlen("Unknow command\n"), 0);
  }
}

/*
**	get the next command and separe each word in a aray
**	return that aray and fill nb with the number of word
*/
static char		**get_next_cmd(int *nb)
{
  int			i;
  int			ret;
  char			buff[BUFFER_SIZE];
  char			**param = NULL;

  i = 0;
  send(clnt_socket, "flowstat>", strlen("flowstat>"), 0);
  param = xmalloc(3 * sizeof(*param));
  memset(param, 0, 3 * sizeof(*param));
  memset(buff, 0, BUFFER_SIZE);
  /* read inf */
  /* epur str */
  ret = recv(clnt_socket, buff, BUFFER_SIZE, 0);
  if (ret < 0)
    return (NULL);
  buff[ret - 2] = 0;		/* to remove \n at the end */
  param[i] = strtok(buff, " ");
  if (param[i] != NULL)
    param[i] = strdup(param[i]);
  i++;
  while (i < 2 && (param[i] = strtok(NULL, " ")) != NULL) {
    param[i] = strdup(param[i]);
    i++;
  }
  *nb = i - 1;
  return (param);
}

/*
**	as long as the client is connected get the next commmand
**	and treat it in handle_cmd()
*/
static void		recv_from_client(void)
{
  int			nb;
  char			**param = NULL;

  while ((param = get_next_cmd(&nb)) != NULL) {
      handle_cmd(nb, param);
      free_tab(param);
  }
  close(clnt_socket);
}

/*
**	main fonction of client_handler thread
**	create a server socket and then bind it
**	listen accept only one pending client at once
**	after accepting the client revc_from_client take the hand
*/
void		 	*client_handler(void *ptr)
{
  socklen_t		clnt_addr_len;
  struct sockaddr_in	serv_addr;
  struct sockaddr_in	clnt_addr;
  (void)ptr;

  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL))
    {
      flowstat_perror("pthread_setcanceltype");
      exit(EXIT_FAILURE);
    }
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(5454);
  if ((serv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    flowstat_perror("socket");
    exit(EXIT_FAILURE);
  }
  if (bind(serv_socket, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
    flowstat_perror("bind");
    exit(EXIT_FAILURE);
  }
  if (listen(serv_socket, 1) < 0) {
    flowstat_perror("listen");
    exit(EXIT_FAILURE);
  }
  clnt_addr_len = sizeof(clnt_addr);
  while (1) {
    if ((clnt_socket = accept(serv_socket, (struct sockaddr *) &clnt_addr, &clnt_addr_len)) < 0) {
      flowstat_perror("accept");
      exit(EXIT_FAILURE);
    }
    recv_from_client();
  }
  /* never reached */
  pthread_exit(NULL);
}
