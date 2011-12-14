/*
** packet_handler.c for flowstat in /home/jonathan/Projets/test
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Thu Nov 10 09:54:00 2011 Jonathan Machado
** Last update Tue Dec 13 16:43:19 2011 Jonathan Machado
*/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/icmp.h>
#include <netinet/if_ether.h>
#include <string.h>
#include <time.h>
#include "flowstat.h"

int	       	callback(struct nflog_g_handle *gh, struct nfgenmsg *nfmsg,
			 struct nflog_data *nfa, void *data)
{
  global_info	*info;
  (void)gh;
  (void)nfmsg;

  /*
  ** in order to use the thread pool, nfa must be copied
  ** because the next call of this callback will overwrite nfa
  ** and all the thread will lost their current data
  */

  /* g_thread_pool_push(info->packet_handler, (Here the copy of nfa), NULL); */
  info = data;
  packet_handler(nfa, info);
  return (0);
}

static void		*message_data(void *data)
{
  char			*payload;
  int			len;
  u_int32_t		indev;
  u_int32_t		outdev;
  connection_t		cnt;
  struct iphdr		*iph = NULL;
  struct tcphdr		*tcph = NULL;
  struct {
    u_int8_t	fin;
    u_int8_t	ack;
    u_int8_t	rst;
    u_int32_t	idx;
    peer_t	*peer;
  }			*ret;

  outdev = nflog_get_outdev(data);
  indev = nflog_get_indev(data);
  ret = malloc(sizeof(*ret));
  ret->idx = (outdev ? outdev : indev);
  len = nflog_get_payload(data, &payload);
  ret->peer = malloc(sizeof(*ret->peer));
  memset(ret->peer, 0, sizeof(*ret->peer));
  memset(&cnt, 0, sizeof(cnt));
  iph = (struct iphdr*)payload;
  if (outdev == 0) {
    ret->peer->ips.haddr = iph->daddr;
    ret->peer->ips.paddr = iph->saddr;
    cnt.in_data = len;
    cnt.in_packet = 1;
  } else {
    ret->peer->ips.haddr = iph->saddr;
    ret->peer->ips.paddr = iph->daddr;
    cnt.out_data = len;
    cnt.out_packet = 1;
  }
  cnt.status = ESTABLISHED;
  ret->peer->connections = new_list();
  ret->peer->stat.history = new_list();
  switch (iph->protocol) {
  case IPPROTO_TCP:
    tcph = (struct tcphdr*)((u_int32_t *)iph + iph->ihl);
    if (outdev == 0)
      cnt.port = ntohs(tcph->source);
    else
      cnt.port =  ntohs(tcph->dest);
    cnt.first_packet = time(NULL);
    cnt.last_packet = cnt.first_packet;
    push_front(ret->peer->connections, new_link_by_param(&cnt, sizeof(cnt)));
    ret->fin = tcph->fin;
    ret->ack = tcph->ack;
    ret->rst = tcph->rst;
    break;
  case IPPROTO_UDP:
    ret->peer->stat.udp = 1;
    break;
  case IPPROTO_ICMP:
    ret->peer->stat.icmp = 1;
    /* check if type == destination unreachable, and incr stat.ko ? */
    break;
  default:
    ret->peer->stat.other = 1;
    break;
  }
  return (ret);
}

void	       	packet_handler(void *data, global_info *info)
{
  struct nfulnl_msg_packet_hdr *ph;
  message_queue_t	*message = NULL;;

  message = malloc(sizeof(*message));
  message->type = ADD_PACKET;
  if (!(ph = nflog_get_msg_packet_hdr(data))) {
    flowstat_perror("nflog_get_msg_packet_hdr");
    return ;
  }
  switch (ntohs(ph->hw_protocol)) {
  case 0: 			/* seems that netfilter not always sets the hw_protocol */
  case ETH_P_IP:
    message->data = message_data(data);
    break;
  case ETH_P_IPV6:
    /*
    ** good example on of how to handle this on
    ** http://vuurmuur.svn.sourceforge.net/viewvc/vuurmuur/trunk/vuurmuur/vuurmuur_log/nflog.c
    */
  default:
    break;
  }
  /* free(data); */
  g_async_queue_push(info->packet_queue, message);
}

void		read_and_analyze(global_info *info)
{
  int		len;
  char		buffer[4096];

  len = 0;
  while (!info->finish) {
    len = recv(info->netlink_fd, buffer, sizeof(buffer), 0);
    if (len < 0) {
      flowstat_perror("recv");
      info->finish = 1;
      exit(EXIT_FAILURE);
    } else if (len > 0) {
      nflog_handle_packet(info->handle, buffer, len);
    }
  }
  g_thread_pool_free(info->packet_handler, FALSE, TRUE);
  g_async_queue_unref(info->packet_queue);
  nflog_unbind_group(info->g_handle);
  nflog_close(info->handle);
}
