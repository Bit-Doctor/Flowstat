/*
** header_handler.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep  7 14:28:37 2011 Jonathan Machado
** Last update Tue Nov 22 16:26:58 2011 Jonathan Machado
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

static void    		update_flux(flux *cur_flux, packet_info *pkt_info)
{
  cur_flux->last_packet = pkt_info->time;
  if (pkt_info->protocol == IPPROTO_TCP) {
    if (pkt_info->fin)
      cur_flux->protocol_data.tcp.stts++;
    else if (cur_flux->protocol_data.tcp.stts == wait_last_ack && pkt_info->ack)
      cur_flux->protocol_data.tcp.stts = closed;
    else if (cur_flux->protocol_data.tcp.stts == one_peer_closed && pkt_info->rst)
      cur_flux->protocol_data.tcp.stts = closed;
    else if (pkt_info->rst)
      cur_flux->protocol_data.tcp.stts = reseted;
  }
  if (pkt_info->protocol == IPPROTO_TCP || pkt_info->protocol == IPPROTO_UDP) {
    if (pkt_info->input)
      cur_flux->input_data += pkt_info->data;
    else
      cur_flux->output_data += pkt_info->data;
  }
  if (pkt_info->input)
    cur_flux->input_packet++;
  else
    cur_flux->output_packet++;
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
    pkt_info->port = ntohs(tcph->source);
  else
    pkt_info->port = ntohs(tcph->dest);
  if (tcph->fin)
    pkt_info->fin = 1;
  if (tcph->rst)
    pkt_info->rst = 1;
  if (tcph->ack)
    pkt_info->ack = 1;
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

/*
** fill packet_info struct with raw data from ulog_packet_msg_t
*/
static packet_info     	*get_packet_information(ulog_packet_msg_t *pkt)
{
  packet_info	       	*pkt_info = NULL;
  struct iphdr	       	*iph = NULL;

  pkt_info = xmalloc(sizeof(*pkt_info));
  memset(pkt_info, 0, sizeof(*pkt_info));
  iph = (struct iphdr *)pkt->payload;
  pkt_info->ip = ntohl(iph->daddr);
  pkt_info->data = ntohs(iph->tot_len);
  if (ntohl(iph->daddr) == info.local_ip) {
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

static void	       	create_new_flux(connection *cur_cnt, packet_info *pkt_info)
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
  add_flux_to_end_list(&cur_cnt->head,&cur_cnt->tail, new);
  cur_cnt->number_flow++;
}

static void	       	create_new_connection(packet_info *pkt_info)
{
  connection	       	*new = NULL;
  struct sockaddr_in   	socket;
  char		       	dns[1024];

  if (info.number_connection < info.options.ip_limit) {
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
  } else {
    /* flush and syslog */
  }
}

/*
** fill pkt_info from pkt
** then compare the info with the connections if already listed
** if the connection is listed search if the flux is in the list
** else add them in the linked list
*/
static void	       	packet_handler(ulog_packet_msg_t *pkt)
{
  packet_info  		*pkt_info = NULL;
  connection   		*listed_cnt = NULL;
  flux	       		*listed_flux = NULL;

  pkt_info = get_packet_information(pkt);
  if ((listed_cnt = ip_already_listed(pkt_info->ip)) != NULL) {
    pthread_mutex_lock(&listed_cnt->lock);
    if ((listed_flux = flux_already_listed(listed_cnt, pkt_info)) != NULL)
      update_flux(listed_flux, pkt_info);
    else
      create_new_flux(listed_cnt, pkt_info);
    pthread_mutex_unlock(&listed_cnt->lock);
  } else {
    create_new_connection(pkt_info);
  }
  free(pkt_info);
  pkt_info = NULL;
}

/*
**	main fonction of the packet handler thread
**	read and fill packet sent by the kernel in the
**	libipulog fonctions ipulog_read() and ipulog_get_packet()
**	and call packet handler for the logging
*/
void			*read_and_analyze(void *ptr)
{
  int			len;
  ulog_packet_msg_t	*ulog_packet = NULL;
  struct ipulog_handle	*h = NULL;

  h = ptr;
  if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL))
    {
      flowstat_perror("pthread_setcanceltype");
      exit(EXIT_FAILURE);
    }
  while (1) {
    while (info.connection && (len = ipulog_read(h, info.buffer, BUFFER_SIZE, 1))) {
      if (len <= 0) {
	break;
      }
      while ((ulog_packet = ipulog_get_packet(h, info.buffer, len))) {
	packet_handler(ulog_packet);
      }
    }
  }
  /* never reached */
  pthread_exit(NULL);
}

