/*
** header_handler.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep  7 14:28:37 2011 Jonathan Machado
** Last update Tue Sep 20 12:35:25 2011 Jonathan Machado
*/

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include "libipulog/libipulog.h"
#include "flowstat.h"

extern struct global_info	info;

#ifndef DEBUG	/* if the define debug is not set, all packet are loged in json tree */
		/* if not, fonction of packet_handler_debug.c are used insted */

static void			incr_connection_object(connection *current_connection, packet_info *pkt_info)
{
  current_connection->last_packet = pkt_info->time;
  if (pkt_info->protocol == IPPROTO_TCP && pkt_info->fin)
    current_connection->protocol_data.tcp.stts++;
  if (pkt_info->protocol == IPPROTO_TCP && pkt_info->rst)
    current_connection->protocol_data.tcp.stts = reseted;
  if (pkt_info->protocol == IPPROTO_TCP || pkt_info->protocol == IPPROTO_UDP)
    {
      if (pkt_info->input)
  	  current_connection->input_data += pkt_info->data;
      else
  	  current_connection->output_data += pkt_info->data;
    }
  if (pkt_info->input)
    current_connection->input_packet++;
  else
    current_connection->output_packet++;
}

static int			is_the_same_connection(connection *current_connection, packet_info *pkt_info)
{
  if (pkt_info->protocol != current_connection->protocol)
    return (0);
  switch (pkt_info->protocol)
    {
    case IPPROTO_ICMP:
      if (pkt_info->type != current_connection->protocol_data.icmp.type)
	return (0);
      break;
    case IPPROTO_TCP:
      if (current_connection->protocol_data.tcp.stts == closed || current_connection->protocol_data.tcp.stts == reseted)
	return (0);
      if (pkt_info->input && pkt_info->port != current_connection->protocol_data.tcp.port)
	return (0);
      else if (!pkt_info->input && pkt_info->port != current_connection->protocol_data.tcp.port)
	return (0);
      break;
    case IPPROTO_UDP:
      if (pkt_info->input && pkt_info->port !=  current_connection->protocol_data.udp.port)
	return (0);
      else if (!pkt_info->input && pkt_info->port != current_connection->protocol_data.udp.port)
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
  if (tcph->fin)
    pkt_info->fin = 1;
  if (tcph->rst)
    pkt_info->rst = 1;
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

static void			create_new_connection_object(flux *current_flux, packet_info *pkt_info)
{
  connection			*new;

  new = xmalloc(sizeof(*new));
  new->protocol = pkt_info->protocol;
  switch (pkt_info->protocol)
    {
    case IPPROTO_ICMP:
      new->protocol_data.icmp.type = pkt_info->type;
      break;
    case IPPROTO_TCP:
      new->protocol_data.tcp.port = pkt_info->port;
      new->protocol_data.tcp.stts = connected;
      break;
    case IPPROTO_UDP:
      new->protocol_data.udp.port = pkt_info->port;
      break;
    }
  if (pkt_info->input)
    {
      new->input_data = pkt_info->data;
      new->input_packet = 1;
    }
  else
    {
      new->output_data = pkt_info->data;
      new->output_packet = 1;
    }
  new->first_packet = pkt_info->time;
  new->last_packet = pkt_info->time;
  new->next = NULL;
  if (current_flux->head == NULL)
    {
      current_flux->head = new;
      current_flux->tail = new;
    }
  else
    {
      current_flux->tail->next = new;
      current_flux->tail = new;
    }
  current_flux->number_connections++;
}

static void			create_new_flux_object(packet_info *pkt_info)
{
  flux			*new;
#ifdef DNS_ACTIVATE
  struct sockaddr_in	socket;
  char			dns[1024];

  socket.sin_family = AF_INET;
  socket.sin_addr.s_addr = htonl(pkt_info->ip);
  socket.sin_port = htons(80);
  getnameinfo(&socket, sizeof(socket), dns, sizeof(dns), NULL, 0, 0);
#endif /* DNS_ACTIVATE */
  new = xmalloc(sizeof(*new));
  new->ip = pkt_info->ip;
#ifdef DNS_ACTIVATE
  new->hostname = strdup(dns);
#endif /* DNS_ACTIVATE */
  new->number_connections = 0;
  new->head = NULL;
  new->tail = NULL;
  new->next = NULL;
  if (info.head == NULL)
    {
      info.head = new;
      info.tail = new;
    }
  else
    {
      info.tail->next = new;
      info.tail = new;
    }
  info.number_flux++;
  create_new_connection_object(new, pkt_info);
}

static flux		*ip_already_listed(packet_info *pkt_info)
{
  flux				*current;

  current = info.head;
  while (current != NULL)
    {
      if (pkt_info->ip == current->ip)
	return (current);
      current = current->next;
    }
  return (NULL);
}

static connection		*connection_already_listed(flux *current_flux, packet_info *pkt_info)
{
  connection			*current;

  current = current_flux->head;
  while (current)
    {
      if (is_the_same_connection(current, pkt_info))
	return (current);
      current = current->next;
    }
  return (NULL);
}

void			packet_handler(ulog_packet_msg_t *pkt)
{
  packet_info		*pkt_info;
  flux			*listed_flux;
  connection		*listed_connection;

  pkt_info = get_packet_information(pkt);
  if ((listed_flux = ip_already_listed(pkt_info)) != NULL)
    {
      if ((listed_connection = connection_already_listed(listed_flux, pkt_info)) != NULL)
	incr_connection_object(listed_connection, pkt_info);
      else
	create_new_connection_object(listed_flux, pkt_info);
    }
  else
    create_new_flux_object(pkt_info);
  free(pkt_info);
}

#endif /* DEBUG */
