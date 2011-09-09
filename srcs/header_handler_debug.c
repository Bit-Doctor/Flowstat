/*
** header_handler_debug.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Thu Sep  8 17:18:41 2011 Jonathan Machado
** Last update Fri Sep  9 12:59:06 2011 Jonathan Machado
*/

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include "cson_amalgamation_core.h"
#include "libipulog/libipulog.h"
#include "flowstat.h"

extern cson_value		*rootV;
extern cson_object		*root;
extern cson_array		*ips;

#ifdef	DEBUG
/*
** if the define debug is set, all packet information are printed on stdout
** if not, fonction of header_handler.c are used
*/

void			icmpheader_handler(void *protocol_header)
{
  struct icmphdr *icmph;

  icmph = protocol_header;
  printf("type :");
  switch(icmph->type)
    {
    case ICMP_ECHO:
      printf("echo\n");
      break;
    case ICMP_ECHOREPLY:
      printf("echo reply\n");
      break;
    case ICMP_DEST_UNREACH:
      printf("destination unreachable\n");
      break;
    default:
      printf("other : %u\n", icmph->type);
    }
}

void			tcpheader_handler(void *protocol_header)
{
  struct tcphdr		*tcph;

  tcph = protocol_header;
  printf("port source: %u\n", ntohs(tcph->source));
  printf("port destination: %u\n", ntohs(tcph->dest));
  printf("flag :");
  if (tcph->ack)
    printf(" ACK ");
  if (tcph->syn)
    printf(" SYN ");
  if (tcph->fin)
    printf(" FIN ");
  printf("\n");
}

void			udpheader_handler(void *protocol_header)
{
  struct udphdr		*udph;

  udph = protocol_header;
  printf("port source: %u\n", ntohs(udph->source));
  printf("port destination: %u\n", ntohs(udph->dest));
}

void			packet_handler(ulog_packet_msg_t *pkt)
{
  u_int32_t		 addr;
  struct iphdr		*iph;

  iph = (struct iphdr *)pkt->payload;
  addr = ntohl(iph->saddr);
  printf("ip source: %u.%u.%u.%u\n", INTTOIP(addr));
  addr = ntohl(iph->daddr);
  printf("ip destination: %u.%u.%u.%u\n", INTTOIP(addr));
  printf("protocol: ");
  switch (iph->protocol)
    {
    case IPPROTO_ICMP:
      printf("icmp\n");
      icmpheader_handler((u_int32_t *)iph + iph->ihl);
      break;
    case IPPROTO_TCP:
      printf("tcp\n");
      tcpheader_handler((u_int32_t *)iph + iph->ihl);
      break;
    case IPPROTO_UDP:
      printf("udp\n");
      udpheader_handler((u_int32_t *)iph + iph->ihl);
      break;
    default:
      printf("%u\n", iph->protocol);
    }
  printf("-------------------------------\n");
}

#endif /* DEBUG */
