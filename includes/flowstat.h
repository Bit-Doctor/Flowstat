/*
** flowstat.h for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep  7 14:24:24 2011 Jonathan Machado
** Last update Mon Sep 19 11:08:22 2011 Jonathan Machado
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

/* Size of the receive buffer for the netlink socket.  Should be at least the same size as the
 * 'nlbufsiz' module loadtime parameter of ipt_ULOG.o
 * If you have _big_ in-kernel queues, you may have to increase this number.  (
 * --qthreshold 100 * 1500 bytes/packet = 150kB  */
# define ULOGD_RMEM_DEFAULT	150000
# define BUFFER_SIZE		2048
# define GROUP_NETLINK		1
# define LINKED			1
# define ONE_PEER_CLOSE		2
# define CLOSED			3
# define RESETED		4
# define INTTOIP(addr) ((unsigned char *)&addr)[3], ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[1], ((unsigned char *)&addr)[0]

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

struct		global_info
{
  int			local_ip;
  char			*buffer;
  struct ipulog_handle	*connection;
  cson_value		*rootV;
  cson_object		*root;
  cson_array		*flux;
};

/*
**
**		PROTOTYPE
**
*/

struct ipulog_handle    *verified_ipulog_create_handle(u_int32_t group_mask, u_int32_t rcvbufsize);
void			*xmalloc(int size);
void			packet_handler(ulog_packet_msg_t *pkt);

#endif
