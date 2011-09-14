/*
** header_handler.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep  7 14:28:37 2011 Jonathan Machado
** Last update Wed Sep 14 13:43:59 2011 Jonathan Machado
*/

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include "cson_amalgamation_core.h"
#include "libipulog/libipulog.h"
#include "flowstat.h"

extern struct global_info	info;

#ifndef DEBUG
/*
** if the define debug is not set, all packet are loged in json tree
** if not, fonction of header_handler_debug.c are used
*/

static void			icmpheader_handler(void *protocol_header, cson_object *new)
{
  struct icmphdr *icmph;

  icmph = protocol_header;
  switch(icmph->type)
    {
    case ICMP_ECHO:
      cson_object_set(new, "type", cson_value_new_string("echo", strlen("echo")));
      cson_object_set(new, "type number", cson_value_new_integer(icmph->type));
      break;
    case ICMP_ECHOREPLY:
      cson_object_set(new, "type", cson_value_new_string("echo reply", strlen("echo reply")));
      cson_object_set(new, "type number", cson_value_new_integer(icmph->type));
      break;
    case ICMP_DEST_UNREACH:
      cson_object_set(new, "type", cson_value_new_string("destination unreachable", strlen("destination unreachable")));
      cson_object_set(new, "type number", cson_value_new_integer(icmph->type));
      break;
    default:
      cson_object_set(new, "type", cson_value_new_string("other", strlen("other")));
      cson_object_set(new, "type number", cson_value_new_integer(icmph->type));
    }
}

static void			tcpheader_handler(void *protocol_header, cson_object *new)
{
  struct tcphdr		*tcph;

  tcph = protocol_header;
  cson_object_set(new, "port source", cson_value_new_integer(ntohs(tcph->source)));
  cson_object_set(new, "port destination", cson_value_new_integer(ntohs(tcph->dest)));
}

static void			udpheader_handler(void *protocol_header, cson_object *new)
{
  struct udphdr		*udph;

  udph = protocol_header;
  cson_object_set(new, "port source", cson_value_new_integer(ntohs(udph->source)));
  cson_object_set(new, "port destination", cson_value_new_integer(ntohs(udph->dest)));
}

static void			create_new_connection_object(cson_array *connections,  struct iphdr *iph)
{
  int			addr;
  char			*daddr;
  cson_value		*newV;
  cson_object		*new;

  addr = ntohl(iph->daddr);
  daddr = xmalloc(INET6_ADDRSTRLEN * sizeof(*daddr));
  sprintf(daddr,"%u.%u.%u.%u", INTTOIP(addr));
  newV = cson_value_new_object();
  new = cson_value_get_object(newV);
  cson_object_set(new, "ip destination", cson_value_new_string(daddr, strlen(daddr)));
  cson_object_set(new, "protocole number", cson_value_new_integer(iph->protocol));
  switch (iph->protocol)
    {
    case IPPROTO_ICMP:
      cson_object_set(new, "protocole name", cson_value_new_string("icmp", strlen("icmp")));
      icmpheader_handler((u_int32_t *)iph + iph->ihl, new);
      break;
    case IPPROTO_TCP:
      cson_object_set(new, "protocole name", cson_value_new_string("tcp", strlen("tcp")));
      tcpheader_handler((u_int32_t *)iph + iph->ihl, new);
      break;
    case IPPROTO_UDP:
      cson_object_set(new, "protocole name", cson_value_new_string("udp", strlen("udp")));
      udpheader_handler((u_int32_t *)iph + iph->ihl, new);
      break;
    default:
      cson_object_set(new, "protocole name", cson_value_new_string("other", strlen("other")));
    }
  cson_object_set(new, "first log", cson_value_new_integer(time(NULL)));
  cson_object_set(new, "last log", cson_value_new_integer(time(NULL)));
  cson_object_set(new, "number of occurancy", cson_value_new_integer(1));
  cson_array_append(connections, newV);
  free(daddr);
}

static void			create_new_flux_object(char *saddr, struct iphdr *iph)
{
  cson_value		*newV;
  cson_object		*new;
  cson_value		*connectionsV;
  cson_array		*connections;
#ifdef DNS_ACTIVATE
  struct sockaddr_in	socket;
  char			dns[1024];

  socket.sin_family = AF_INET;
  socket.sin_addr.s_addr = inet_addr(saddr);
  socket.sin_port = htons(80);
  getnameinfo(&socket, sizeof(socket), dns, sizeof(dns), NULL, 0, 0);
#endif
  newV = cson_value_new_object();
  new = cson_value_get_object(newV);
  connectionsV = cson_value_new_array();
  connections = cson_value_get_array(connectionsV);
  cson_object_set(new, "ip source", cson_value_new_string(saddr, strlen(saddr)));
#ifdef DNS_ACTIVATE
  cson_object_set(new, "hostname", cson_value_new_string(dns, strlen(dns)));
#endif
  cson_object_set(new, "first connection", cson_value_new_integer(time(NULL)));
  cson_object_set(new, "last connection", cson_value_new_integer(time(NULL)));
  cson_object_set(new, "connections", connectionsV);
  cson_array_append(info.flux, newV);
  create_new_connection_object(connections, iph);
}

static void			incr_connection_object(cson_object* object)
{
  int			prev_occ;
  char			*date;
  time_t		t;

  time(&t);
  date = ctime(&t);
  date[24] = 0;
  prev_occ = cson_value_get_integer(cson_object_get(object, "number of occurancy"));
  cson_object_set(object, "last log", cson_value_new_integer(time(NULL)));
  cson_object_set(object, "number of occurancy", cson_value_new_integer(prev_occ + 1));
}

static int			is_connection_already_listed(cson_object* object,  struct iphdr *iph)
{
  void				*protocol_header;
  struct tcphdr			*tcph;
  struct icmphdr		*icmph;
  struct udphdr			*udph;

  if (iph->protocol != cson_value_get_integer(cson_object_get(object, "protocole number")))
    return (0);
  protocol_header = (u_int32_t *)iph + iph->ihl;
  switch (iph->protocol)
    {
    case IPPROTO_ICMP:
      icmph = protocol_header;
      if (icmph->type != cson_value_get_integer(cson_object_get(object, "type number")))
	return (0);
      break;
    case IPPROTO_TCP:
      tcph = protocol_header;
      if (htons(tcph->source) != cson_value_get_integer(cson_object_get(object, "port source")) ||
	  htons(tcph->dest) != cson_value_get_integer(cson_object_get(object, "port destination")))
	return (0);
      break;
    case IPPROTO_UDP:
      udph = protocol_header;
      if (htons(udph->source) != cson_value_get_integer(cson_object_get(object, "port source")) ||
	  htons(udph->dest) != cson_value_get_integer(cson_object_get(object, "port destination")))
	return (0);
    }
  return (1);
}


void			packet_handler(ulog_packet_msg_t *pkt)
{
  int			i;
  int			j;
  int			find;
  int			flux_len;
  int			connections_len;
  int			addr;
  char			*saddr;
  struct iphdr		*iph;
  cson_object		*tempO;
  cson_array		*tempA;

  iph = (struct iphdr *)pkt->payload;
  saddr = xmalloc(INET6_ADDRSTRLEN * sizeof(*saddr));
  addr = ntohl(iph->saddr);
  sprintf(saddr,"%u.%u.%u.%u", INTTOIP(addr));
  flux_len = cson_array_length_get(info.flux);
  for (i = 0, find = 0; i < flux_len; ++i)
    {
      tempO = cson_value_get_object(cson_array_get(info.flux, i));
      if (strcmp(cson_string_cstr(cson_value_get_string(cson_object_get(tempO, "ip source"))), saddr) == 0)
	{
	  cson_object_set(tempO, "last connection", cson_value_new_integer(time(NULL)));
	  tempA =  cson_value_get_array(cson_object_get(tempO, "connections"));
	  connections_len = cson_array_length_get(tempA);
	  for (j = 0; j < connections_len; ++j)
	    {
	      tempO = cson_value_get_object(cson_array_get(tempA, j));
	      if (is_connection_already_listed(tempO, iph))
		{
		  incr_connection_object(tempO);
		  find = 1;
		}
	    }
	  if (!find)
	    create_new_connection_object(tempA, iph);
	  find = 1;
	}
    }
  if (!find)
    create_new_flux_object(saddr, iph);
  free(saddr);
}

#endif /* DEBUG */
