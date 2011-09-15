/*
** header_handler.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep  7 14:28:37 2011 Jonathan Machado
** Last update Thu Sep 15 16:51:05 2011 Jonathan Machado
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

static void			incr_connection_object(cson_object* ip_flux, cson_object *connection, packet_info *pkt_info)
{
  int				prev_occ;
  int				prev_data;

  if (pkt_info->protocol == IPPROTO_TCP || pkt_info->protocol == IPPROTO_UDP)
    {
      if (pkt_info->input)
  	{
  	  prev_data = cson_value_get_integer(cson_object_get(connection, "input data"));
  	  cson_object_set(connection, "input data", cson_value_new_integer(prev_data + pkt_info->data));
  	}
      else
  	{
  	  prev_data = cson_value_get_integer(cson_object_get(connection, "output data"));
  	  cson_object_set(connection, "output data", cson_value_new_integer(prev_data + pkt_info->data));
  	}
    }
  cson_object_set(connection, "last packet", cson_value_new_integer(pkt_info->time));
  cson_object_set(ip_flux, "last connection", cson_value_new_integer(pkt_info->time));
  if (pkt_info->input)
    {
      prev_occ = cson_value_get_integer(cson_object_get(connection, "number of input packet"));
      cson_object_set(connection, "number of input packet", cson_value_new_integer(prev_occ + 1));
    }
  else
    {
      prev_occ = cson_value_get_integer(cson_object_get(connection, "number of output packet"));
      cson_object_set(connection, "number of output packet", cson_value_new_integer(prev_occ + 1));
    }
}

static int			is_the_same_connection(cson_object* object, packet_info *pkt_info)
{
  if (pkt_info->protocol != cson_value_get_integer(cson_object_get(object, "protocole")))
    return (0);
  switch (pkt_info->protocol)
    {
    case IPPROTO_ICMP:
      if (pkt_info->type != cson_value_get_integer(cson_object_get(object, "type")))
	return (0);
      break;
    case IPPROTO_TCP:
      if (pkt_info->input && pkt_info->port != cson_value_get_integer(cson_object_get(object, "port")))
	return (0);
      else if (!pkt_info->input && pkt_info->port != cson_value_get_integer(cson_object_get(object, "port")))
	return (0);
      break;
    case IPPROTO_UDP:
      if (pkt_info->input && pkt_info->port != cson_value_get_integer(cson_object_get(object, "port")))
	return (0);
      else if (!pkt_info->input && pkt_info->port != cson_value_get_integer(cson_object_get(object, "port")))
	return (0);
    }
  return (1);
}

static void			icmpheader_handler(void *protocol_header, packet_info *pkt_info)
{
  struct icmphdr		*icmph;

  icmph = protocol_header;
  pkt_info->type = icmph->type;
}

static void			tcpheader_handler(void *protocol_header, packet_info *pkt_info)
{
  struct tcphdr			*tcph;

  tcph = protocol_header;
  if (pkt_info->input)
    pkt_info->port = ntohs(tcph->source);
  else
    pkt_info->port = ntohs(tcph->dest);
}

static void			udpheader_handler(void *protocol_header, packet_info *pkt_info)
{
  struct udphdr			*udph;

  udph = protocol_header;
  if (pkt_info->input)
    pkt_info->port = ntohs(udph->source);
  else
    pkt_info->port = ntohs(udph->dest);
}

static packet_info		*get_packet_information(ulog_packet_msg_t *pkt)
{
  packet_info			*pkt_info;
  struct iphdr			*iph;

  pkt_info = xmalloc(sizeof(*pkt_info));
  iph = (struct iphdr *)pkt->payload;
  pkt_info->ip = ntohl(iph->daddr);
  pkt_info->data = ntohs(iph->tot_len);
  if (ntohl(iph->daddr) == info.local_ip)
    {
      pkt_info->ip = ntohl(iph->saddr);
      pkt_info->input = 1;
    }
  pkt_info->protocol = iph->protocol;
  pkt_info->time = time(NULL);
  switch (iph->protocol)
    {
    case IPPROTO_ICMP:
      icmpheader_handler((u_int32_t *)iph + iph->ihl, pkt_info);
      break;
    case IPPROTO_TCP:
      tcpheader_handler((u_int32_t *)iph + iph->ihl, pkt_info);
      break;
    case IPPROTO_UDP:
      udpheader_handler((u_int32_t *)iph + iph->ihl, pkt_info);
      break;
    }
  return (pkt_info);
}

static void			create_new_connection_object(cson_object *ip_flux, packet_info *pkt_info)
{
  cson_value			*newV;
  cson_object			*new;
  cson_array			*connections;

  newV = cson_value_new_object();
  new = cson_value_get_object(newV);
  connections = cson_value_get_array(cson_object_get(ip_flux, "connections"));
  cson_object_set(ip_flux, "last connection", cson_value_new_integer(pkt_info->time));
  cson_object_set(new, "protocole", cson_value_new_integer(pkt_info->protocol));
  switch (pkt_info->protocol)
    {
    case IPPROTO_ICMP:
      cson_object_set(new, "type", cson_value_new_integer(pkt_info->type));
      break;
    case IPPROTO_TCP:
      cson_object_set(new, "port", cson_value_new_integer(pkt_info->port));
      if (pkt_info->input)
	cson_object_set(new, "input data", cson_value_new_integer(pkt_info->data));
      else
	cson_object_set(new, "output data", cson_value_new_integer(pkt_info->data));
      break;
    case IPPROTO_UDP:
      cson_object_set(new, "port", cson_value_new_integer(pkt_info->port));
      if (pkt_info->input)
	cson_object_set(new, "input data", cson_value_new_integer(pkt_info->data));
      else
	cson_object_set(new, "output data", cson_value_new_integer(pkt_info->data));
      break;
    }
  if (pkt_info->input)
    cson_object_set(new, "number of input packet", cson_value_new_integer(1));
  else
    cson_object_set(new, "number of output packet", cson_value_new_integer(1));
  cson_object_set(new, "first packet", cson_value_new_integer(pkt_info->time));
  cson_object_set(new, "last packet", cson_value_new_integer(pkt_info->time));
  cson_array_append(connections, newV);
}

static void			create_new_flux_object(packet_info *pkt_info)
{
  cson_value		*newV;
  cson_object		*new;
  cson_value		*connectionsV;
  cson_array		*connections;
#ifdef DNS_ACTIVATE
  struct sockaddr_in	socket;
  char			dns[1024];

  socket.sin_family = AF_INET;
  socket.sin_addr.s_addr = htonl(pkt_info->ip);
  socket.sin_port = htons(80);
  getnameinfo(&socket, sizeof(socket), dns, sizeof(dns), NULL, 0, 0);
#endif /* DNS_ACTIVATE */
  newV = cson_value_new_object();
  new = cson_value_get_object(newV);
  connectionsV = cson_value_new_array();
  connections = cson_value_get_array(connectionsV);
  cson_object_set(new, "ip", cson_value_new_integer(pkt_info->ip));
#ifdef DNS_ACTIVATE
  cson_object_set(new, "hostname", cson_value_new_string(dns, strlen(dns)));
#endif /* DNS_ACTIVATE */
  cson_object_set(new, "first connection", cson_value_new_integer(pkt_info->time));
  cson_object_set(new, "last connection", cson_value_new_integer(pkt_info->time));
  cson_object_set(new, "connections", connectionsV);
  cson_array_append(info.flux, newV);
  create_new_connection_object(new, pkt_info);
}

static cson_object		*ip_already_listed(packet_info *pkt_info)
{
  int			i;
  int			flux_len;
  cson_object		*tempO;

  flux_len = cson_array_length_get(info.flux);
  for (i = 0; i < flux_len; ++i)
    {
      tempO = cson_value_get_object(cson_array_get(info.flux, i));
      if (pkt_info->ip == cson_value_get_integer(cson_object_get(tempO, "ip")))
	return (tempO);
    }
  return (NULL);
}

static cson_object		*connection_already_listed(cson_object *flux, packet_info *pkt_info)
{
  int			i;
  int			connections_len;
  cson_object		*tempO;
  cson_array		*tempA;

  tempA =  cson_value_get_array(cson_object_get(flux, "connections"));
  connections_len = cson_array_length_get(tempA);
  for (i = 0; i < connections_len; ++i)
    {
      tempO = cson_value_get_object(cson_array_get(tempA, i));
      if (is_the_same_connection(tempO, pkt_info))
	return (tempO);
    }
  return (NULL);
}

void			packet_handler(ulog_packet_msg_t *pkt)
{
  packet_info		*pkt_info;
  cson_object		*ip_flux;
  cson_object		*connection;

  pkt_info = get_packet_information(pkt);
  if ((ip_flux = ip_already_listed(pkt_info)) != NULL)
    {
      if ((connection = connection_already_listed(ip_flux, pkt_info)) != NULL)
	incr_connection_object(ip_flux, connection, pkt_info);
      else
	create_new_connection_object(ip_flux, pkt_info);
    }
  else
    create_new_flux_object(pkt_info);
  free(pkt_info);
}

#endif /* DEBUG */
