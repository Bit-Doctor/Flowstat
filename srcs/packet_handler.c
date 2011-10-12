/*
** header_handler.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep  7 14:28:37 2011 Jonathan Machado
** Last update Mon Oct 10 10:50:08 2011 Jonathan Machado
*/

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include "libipulog/libipulog.h"
#include "flowstat.h"

extern struct global_info	info;

static void    		update_flux(flux *current_flux, packet_info *pkt_info)
{
  current_flux->last_packet = pkt_info->time;
  if (pkt_info->protocol == IPPROTO_TCP && pkt_info->fin)
    current_flux->protocol_data.tcp.stts++;
  if (pkt_info->protocol == IPPROTO_TCP && pkt_info->rst)
    current_flux->protocol_data.tcp.stts = reseted;
  if (pkt_info->protocol == IPPROTO_TCP || pkt_info->protocol == IPPROTO_UDP) {
    if (pkt_info->input)
      current_flux->input_data += pkt_info->data;
    else
      current_flux->output_data += pkt_info->data;
  }
  if (pkt_info->input)
    current_flux->input_packet++;
  else
    current_flux->output_packet++;
}

static void    		icmpheader_handler(void *protocol_header, packet_info *pkt_info)
{
  struct icmphdr       	*icmph = NULL;

  icmph = protocol_header;
  pkt_info->type = icmph->type;
}

static void    		tcpheader_handler(void *protocol_header, packet_info *pkt_info)
{
  struct tcphdr	       	*tcph = NULL;

  tcph = protocol_header;
  if (pkt_info->input)
    pkt_info->port = (u_int16_t)ntohs(tcph->source);
  else
    pkt_info->port = ntohs(tcph->dest);
  if (tcph->fin)
    pkt_info->fin = 1;
  if (tcph->rst)
    pkt_info->rst = 1;
}

static void    		udpheader_handler(void *protocol_header, packet_info *pkt_info)
{
  struct udphdr	       	*udph = NULL;

  udph = protocol_header;
  if (pkt_info->input)
    pkt_info->port = ntohs(udph->source);
  else
    pkt_info->port = ntohs(udph->dest);
}

static packet_info     	*get_packet_information(ulog_packet_msg_t *pkt)
{
  packet_info	       	*pkt_info = NULL;
  struct iphdr	       	*iph = NULL;

  /*
  ** fill packet_info strcut with raw data from ulog_packet_msg_t
  **
  */
  pkt_info = xmalloc(sizeof(*pkt_info));
  memset(pkt_info, 0, sizeof(*pkt_info));
  iph = (struct iphdr *)pkt->payload;
  pkt_info->ip = ntohl(iph->daddr);
  pkt_info->data = ntohs(iph->tot_len);
  if (ntohl(iph->daddr) == info.local_ip || ntohl(iph->daddr) == LOCALIP) {
    pkt_info->ip = ntohl(iph->saddr);
    pkt_info->input = 1;
  }
  pkt_info->protocol = iph->protocol;
  pkt_info->time = time(NULL);
  switch (iph->protocol) {
  case IPPROTO_ICMP:
    icmpheader_handler((u_int32_t *)iph + iph->ihl, pkt_info);
    break;
  case IPPROTO_TCP:
    tcpheader_handler((u_int32_t *)iph + iph->ihl, pkt_info);
    break;
  case IPPROTO_UDP:
    udpheader_handler((u_int32_t *)iph + iph->ihl, pkt_info);
    break;
  default:
    break;
  }
  return (pkt_info);
}

static void	       	create_new_flux(connection *current_connection, packet_info *pkt_info)
{
  flux	       		*new = NULL;

  new = xmalloc(sizeof(*new));
  memset(new,0, sizeof(*new));
  new->protocol = pkt_info->protocol;
  switch (pkt_info->protocol) {
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
  default:
    break;
  }
  if (pkt_info->input) {
    new->input_data = pkt_info->data;
    new->input_packet = 1;
  } else {
    new->output_data = pkt_info->data;
    new->output_packet = 1;
  }
  new->first_packet = pkt_info->time;
  new->last_packet = pkt_info->time;
  new->next = NULL;
  if (current_connection->head == NULL)
    current_connection->head = new;
  else
    current_connection->tail->next = new;
  current_connection->tail = new;
  current_connection->number_flow++;
}

static void	       	create_new_connection(packet_info *pkt_info)
{
  connection	       	*new = NULL;
  struct sockaddr_in   	socket;
  char		       	dns[1024];

  if (info.options.dns) {
    socket.sin_family = AF_INET;
    socket.sin_addr.s_addr = htonl(pkt_info->ip);
    socket.sin_port = htons(80);
    getnameinfo((const struct sockaddr *)&socket, sizeof(socket), dns, sizeof(dns), NULL, 0, 0);
  }
  new = xmalloc(sizeof(*new));
  memset(new,0, sizeof(*new));
  new->ip = pkt_info->ip;
  if (info.options.dns)
    new->hostname = strdup(dns);
  new->number_flow = 0;
  new->head = NULL;
  new->tail = NULL;
  new->next = NULL;
  new->stat.last_ko = NULL;
  pthread_mutex_init(&new->lock, NULL);
  if (info.head == NULL)
    info.head = new;
  else
    info.tail->next = new;
  info.tail = new;
  info.number_connection++;
  create_new_flux(new, pkt_info);
}

static void	       	packet_handler(ulog_packet_msg_t *pkt)
{
  packet_info  		*pkt_info = NULL;
  connection   		*listed_connection = NULL;
  flux	       		*listed_flux = NULL;

  /*
  ** fill pkt_info from pkt
  ** then compare the info with the connections if already listed
  ** if the connection is listed search if the flux exist
  ** else add them in the linked list
  */
  pkt_info = get_packet_information(pkt);
  if ((listed_connection = ip_already_listed(pkt_info->ip)) != NULL) {
    pthread_mutex_lock(&listed_connection->lock);
    if ((listed_flux = flux_already_listed(listed_connection, pkt_info)) != NULL)
      update_flux(listed_flux, pkt_info);
    else
      create_new_flux(listed_connection, pkt_info);
    pthread_mutex_unlock(&listed_connection->lock);
  } else {
    create_new_connection(pkt_info);
  }
  free(pkt_info);
  pkt_info = NULL;
}

void			*read_and_analyze(void *ptr)
{
  int			len;
  ulog_packet_msg_t	*ulog_packet = NULL;
  struct ipulog_handle	*h = NULL;

  h = ptr;
  while ((len = ipulog_read(h, info.buffer, BUFFER_SIZE, 1))) {
    while ((ulog_packet = ipulog_get_packet(h, info.buffer, len))) {
      packet_handler(ulog_packet);
    }
  }
  return (NULL);
}

