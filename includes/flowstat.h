/*
** flowstat.h for flowstat in /home/jonathan/Projets/test
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Thu Nov 10 09:55:11 2011 Jonathan Machado
** Last update Mon Dec 19 15:30:40 2011 Jonathan Machado
*/

#ifndef __FLOWSTAT_H__
# define __FLOWSTAT_H__

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <errno.h>
# include <glib.h>
# include <pthread.h>
# include <semaphore.h>
# include <libnetfilter_log/libnetfilter_log.h>
# include "libdatac_list.h"

/*****************************************
**					**
**		  DEFINE		**
**					**
*****************************************/

# define BUFFER_SIZE 4096
# define THREAD_POOL_SIZE 10
typedef int (*CbFunc)(struct nflog_g_handle *, struct nfgenmsg *, struct nflog_data *, void *);

/*****************************************
**					**
**	       	   ENUM   		**
**					**
*****************************************/

typedef enum		status_e
  {
    RESET = -1,
    ESTABLISHED,
    FINWAIT,
    CLOSING,
    CLOSED
  }			status_t;

typedef enum		message_type_e
  {
    ADD_PACKET,
    GET_CONNECTIONS_LIST,
    GET_HISTORY,
    GET_IP_LIST,
    GET_PEER_STAT,
    INCR_UDP,
    INCR_ICMP,
    INCR_OTHER,
    INCR_KO,
    RESET_HASH_TABLE,
    KILL
  }			message_type_t;

/*****************************************
**					**
**		STRUCTURE		**
**					**
*****************************************/

typedef struct		message_queue_s
{
  message_type_t      	type;
  sem_t			semaphore;
  void			*data;
}			message_queue_t;

typedef struct		connection_s
{
  u_int16_t		port;	    /* port number */
  unsigned int	       	in_packet;  /* nb of input packet */
  unsigned int	       	in_data;    /* nb of input data */
  unsigned int	       	out_packet; /* nb of output packet */
  unsigned int	       	out_data; /* nb of output data */
  time_t	       	first_packet;
  time_t		last_packet;
  status_t		status;	/* status of the connection */
}			connection_t;

typedef struct		peer_stat_s
{
  int			udp;	/* nb of udp packet */
  int			icmp;	/* nb of icmp packet */
  int			other;	/* nb of other protocol packet */
  int			tcp;	/* nb of tcp connections */
  int			ko;	/* nb of bad connections/host unreachable */
  List			*history;
}			peer_stat_t;

typedef struct		history_s
{
  u_int16_t		port;
  unsigned long int	nb;
  time_t	       	first;
  time_t		last;
}			history_t;

typedef struct		ip_peer_s
{
  u_int32_t		haddr; 	/* ip host */
  u_int32_t		paddr;	/* ip peer */
}			ip_peer_t;

typedef struct		peer_s
{
  char			*interface;
  char			*hostname;
  ip_peer_t		ips;
  List			*connections;
  peer_stat_t		stat;
}			peer_t;

typedef struct		global_info_s
{
  u_int8_t		finish;	/* flag for end properly the deamon */
  int			netlink_fd; /* fd of the netlink socket */
  struct nflog_handle	*handle;
  struct nflog_g_handle *g_handle;
  GAsyncQueue		*packet_queue;
  GThreadPool		*packet_handler;
  struct {
    u_int32_t	       	max_peer;
    u_int32_t  		history_size;
  }			options;
}			global_info;

typedef struct		cmd_info_s
{
  char			*cmd;
  short			nb_param;
  void			(*f)(global_info *, char **);
}			cmd_info_t;

/*****************************************
**					**
**		PROTOTYPE		**
**					**
*****************************************/

void	packet_handler(void *, global_info *);
void	read_and_analyze(global_info *);
void	hashtable_handler(global_info *);
void   	*client_handler(global_info *);
int	callback(struct nflog_g_handle *, struct nfgenmsg *,
		 struct nflog_data *, void *);
void	free_peer_t(peer_t *);
void   	flowstat_perror(char *);
void   	free_tab(char **);
int	compare_connection(connection_t *, connection_t *);
int	compare_history(history_t *hist, int *port);
char	*get_hostname(u_int32_t ip);
void	free_message_t(message_queue_t *msg);
int    	demonize(void);

#endif	/* __FLOWSTAT_H__*/
