/*
** flowstat.h for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep  7 14:24:24 2011 Jonathan Machado
** Last update Tue Sep 20 12:32:48 2011 Jonathan Machado
*/

#ifndef __FLOWSTAT_H__
# define __FLOWSTAT_H__

/*
**
**		HEADER UTILS
**
*/

# include <errno.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>

# include "cson_amalgamation_core.h"
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
**		STRUCTURE
**
*/

typedef enum	status
  {
    reseted = -1,
    connected,
    one_peer_closed,
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
}		packet_info;

typedef struct        	connection
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
  struct connection	*next;
}     	     		connection;

typedef struct		flux
{
  u_int32_t		ip;
  char			*hostname;
  int			number_connections;
  struct connection	*head;
  struct connection	*tail;
  struct flux		*next;
}			flux;


struct		global_info
{
  int			local_ip;
  char			*buffer;
  struct ipulog_handle	*connection;
  int			number_flux;
  struct flux		*head;
  struct flux		*tail;
};

/*
**
**		PROTOTYPE
**
*/

void			*xmalloc(int);
void			packet_handler(ulog_packet_msg_t*);	/* all the logging system is here */
int			get_local_ip(void);			/* return local ip of INTERFACE */
int			demonize(void);				/* return -1 if another instance is running and 0 if not */
struct ipulog_handle    *verified_ipulog_create_handle(u_int32_t, u_int32_t);

#endif
