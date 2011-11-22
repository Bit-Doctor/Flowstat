/*
** flowstat.h for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep  7 14:24:24 2011 Jonathan Machado
** Last update Tue Nov 22 16:27:10 2011 Jonathan Machado
*/

#ifndef __FLOWSTAT_H__
# define __FLOWSTAT_H__

# include <unistd.h>
# include <pthread.h>
# include "libipulog/libipulog.h"
/*
**
**		DEFINE
**
*/

# define ULOGD_RMEM_DEFAULT	150000	/* Size of the receive buffer for the netlink socket. */
					/* If you have _big_ in-kernel queues, you may have to increase this number.*/
# define BUFFER_SIZE		2048
# define GROUP_NETLINK		1	/* Group link for the connection to iptables, by default 1 */
# define INTERFACE		"eth0"	/* Wich interface flowstat must keep the ip */
# define INTTOIP(addr) ((unsigned char *)&addr)[3], ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[1], ((unsigned char *)&addr)[0]
/*
**
**		ENUM
**
*/

typedef enum	status
  {
    reseted = -1,
    connected,
    one_peer_closed,
    wait_last_ack,
    closed
  }		status;

/*
**
**		STRUCTURE
**
*/

typedef struct	packet_info
{
  u_int32_t		ip;
  u_int16_t		input;
  u_int16_t		protocol;
  time_t		time;
  u_int16_t		port;
  u_int16_t		data;
  u_int8_t		type;
  u_int8_t		fin;
  u_int8_t		rst;
  u_int8_t		ack;
}		packet_info;

struct			flux_stat
{
  u_int32_t		ok;
  u_int32_t		ko;
  u_int32_t		udp;
  u_int32_t		icmp;
  u_int32_t		other;
  struct flux		*last_ko;
  struct flux		*history_head;
  struct flux		*history_tail;
};

typedef struct        	flux
{
  u_int16_t		protocol;
  time_t       	       	first_packet;
  time_t               	last_packet;
  u_int32_t    	       	input_data;
  u_int32_t    		input_packet;
  u_int32_t   	       	output_data;
  u_int32_t    		output_packet;
  union
  {
    struct
    {
      u_int8_t		type;
    }  		        icmp;
    struct
    {
      u_int16_t		port;
    }  			udp;
    struct
    {
      u_int16_t		port;
      status		stts;
    }	       		tcp;
  }    	       	       	protocol_data;
  struct flux		*next;
}     	     		flux;

typedef struct		connection
{
  u_int32_t		ip;
  char			*hostname;
  u_int32_t    		number_flow;
  struct flux_stat	stat;
  struct flux		*head;
  struct flux		*tail;
  struct connection    	*next;
  pthread_mutex_t	lock;
}			connection;

struct			global_info
{
  struct
  {
    char		advanced;
    char		dns;
    u_int32_t  		ip_limit;
    u_int32_t		history_size;
  }			options;
  u_int32_t	       	local_ip;
  unsigned char	       	*buffer;
  struct ipulog_handle	*connection;
  u_int32_t    		number_connection;
  pthread_t		threads[3];
  struct connection    	*head;
  struct connection    	*tail;
};

struct			cmd_info
{
  char			*cmd;
  int			nb_param;
  void			(*f)(char **);
};

/*
**
**		PROTOTYPE
**
*/

void			*xmalloc(int);
void			free_tab(char **tab);
void			free_at_interupt(void);
void		 	*client_handler(void *ptr);
void			delete_flux(connection *current_connection, flux *prev, flux *delete);
void			free_connection_list(connection *head);
void			*read_and_analyze(void *ptr);
void			*flush_and_calc(void *ptr);
void			flush_closed_connection(void);		/* flush closed connection into the stat structure */
void			flowstat_perror(char *str);		/* log error message with syslog/print on stdout if DEGUG is defne */
u_int32_t      		get_local_ip(void);			/* return local ip of INTERFACE */
int			demonize(void);				/* return -1 if another instance is running and 0 if not */
struct ipulog_handle    *verified_ipulog_create_handle(u_int32_t, u_int32_t);
flux			*extract_flux(connection *current_connection, flux *prev, flux *delete);
flux    		*flux_already_listed(connection *current_connection, packet_info *pkt_info);
connection		*ip_already_listed(u_int32_t ip);
void			add_flux_to_end_list(flux **head, flux **tail, flux *new);
void			pop_and_push_flux(flux **head, flux **tail, flux *new);
void			pop_flux(flux **head, flux **tail);
u_int32_t      		size_flux_list(flux *head);

#endif	/* __FLOWSTAT_H__*/
