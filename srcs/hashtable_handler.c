/*
** hashtable_handler.c for flowstat in /home/jonathan/Projets/test
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Thu Nov 10 12:20:38 2011 Jonathan Machado
** Last update Mon Dec 12 14:34:39 2011 Jonathan Machado
*/

#include <net/if.h>
#include "flowstat.h"

static void	update_stat(peer_stat_t *old, peer_stat_t *new)
{
  old->udp += new->udp;
  old->icmp += new->icmp;
  old->other += new->other;
  old->ko += new->ko;
}

static status_t	update_connection(connection_t *old, connection_t *new,
				  u_int8_t ack, u_int8_t fin, u_int8_t rst)
{
  old->in_packet += new->in_packet;
  old->in_data += new->in_data;
  old->out_data += new->out_data;
  old->out_packet += new->out_packet;
  old->last_packet = new->first_packet;
  if (fin == 1)
    old->status++;
  else if (old->status == CLOSING && ack == 1)
    old->status = CLOSED;
  else if (old->status == FINWAIT && rst == 1)
    old->status = CLOSED;
  else if (rst == 1)
    old->status = RESET;
  return (old->status);
}

/*
** if g_hash_table_lookup return NULL, the peer is not listed
** so we add new->peer in the hashtable.
** but if g_hash_table_lookup return a value, the stat are updated
** and if it's a tcp connection we add it in peer->connection if it's a new one
** or update it if not.
*/
static void    	message_add_packet(GHashTable *hshtbl, void *ptr, global_info *info)
{
  status_t	status;
  int		*key = NULL;
  peer_t       	*peer = NULL;
  Link		*link = NULL;
  struct {
    u_int8_t	fin;
    u_int8_t	ack;
    u_int8_t	rst;
    u_int32_t	idx;
    peer_t	*peer;
  }		*new = NULL;

  new = ptr;
  key = malloc(sizeof(*key));
  *key = new->peer->ips.haddr + new->peer->ips.paddr;
  peer = g_hash_table_lookup(hshtbl, key);
  if (peer == NULL) {
    new->peer->hostname = get_hostname(new->peer->ips.paddr);
    new->peer->interface = malloc(IF_NAMESIZE *
  				  sizeof(*new->peer->interface));
    if_indextoname(new->idx, new->peer->interface);
    g_hash_table_insert(hshtbl, key, new->peer);
    if (info->options.max_peer != 0 &&
        g_hash_table_size(hshtbl) > info->options.max_peer)
      g_hash_table_remove_all(hshtbl);	/* flush hashtable */
  } else {
    free(key);
    if (new->peer->connections->head != NULL) {
      link = lookup(peer->connections, new->peer->connections->head->ptr,
  		    (int (*)(void *, void *))&compare_connection);
      if (link == NULL) {
  	push_end(peer->connections, pop_front(new->peer->connections));
      } else {
  	status = update_connection(link->ptr, new->peer->connections->head->ptr,
  				   new->ack, new->fin, new->rst);
  	if (status == CLOSED || status == RESET) {
	  link = lookup_and_pop(peer->connections, link->ptr,
  				(int (*)(void *, void *))&compare_connection);
	  if (status == RESET) {
	    peer->stat.ko++;
	    delete_link(link, &free);
	  } else {
	    peer->stat.tcp++;
	    push_end(peer->stat.history, link);
	    if (peer->stat.history->size > info->options.history_size)
	      delete_link(pop_front(peer->stat.history), &free);
	  }
  	}
      }
    }
    update_stat(&peer->stat, &new->peer->stat);
    free_peer_t(new->peer);
  }
  free(new);
}

/*
** get the connections list and notify the other side
** that he can now reed in message->data
*/
static void   	message_get_connections_list(GHashTable *hshtbl,
					     message_queue_t *message)
{
  peer_t	*peer = NULL;

  peer = g_hash_table_lookup(hshtbl, message->data);
  if (peer != NULL)
    message->data = peer->connections;
  else
    message->data = NULL;
  sem_post(&message->semaphore);
}
/*
** same as message_get_connections_list but fill message->data
** with the ip_list
*/
static void    	message_get_ip_list(GHashTable *hshtbl,
				    message_queue_t *message)
{
  int		i;
  int		len;
  int		*key = NULL;
  peer_t	**list = NULL;
  peer_t       	*value = NULL;
  GHashTableIter iter;

  i = 0;
  len = g_hash_table_size(hshtbl);
  list = malloc((len + 1) * sizeof(*list));
  g_hash_table_iter_init(&iter, hshtbl);
  while (g_hash_table_iter_next(&iter, (void **)&key, (void **)&value))
    {
      list[i] = value;
      i++;
    }
  list[i] = NULL;
  message->data = list;
  sem_post(&message->semaphore);
}

/*
** fill message data with the stat of a peer
*/
static void    	message_get_peer_stat(GHashTable *hshtbl,
				      message_queue_t *message)
{
  peer_t	*peer = NULL;

  peer = g_hash_table_lookup(hshtbl, message->data);
  if (peer != NULL)
    message->data = &peer->stat;
  else
    message->data = NULL;
  sem_post(&message->semaphore);
}

/*
** handle the message in the async queue
*/
void			hashtable_handler(global_info *info)
{
  message_queue_t      	*message = NULL;
  GHashTable		*peer_hshtbl = NULL;

  g_async_queue_ref(info->packet_queue);
  peer_hshtbl = g_hash_table_new_full(g_int_hash, g_int_equal, &free,
				      (GDestroyNotify)&free_peer_t);
  while (!info->finish) {
    message = g_async_queue_pop(info->packet_queue);
    switch (message->type) {
    case ADD_PACKET:
      message_add_packet(peer_hshtbl, message->data, info);
      free(message);
      break;
    case GET_CONNECTIONS_LIST:
      message_get_connections_list(peer_hshtbl, message);
      break;
    case GET_IP_LIST:
      message_get_ip_list(peer_hshtbl, message);
      break;
    case GET_PEER_STAT:
      message_get_peer_stat(peer_hshtbl, message);
      break;
    case RESET_HASH_TABLE:
      g_hash_table_remove_all(peer_hshtbl);
      sem_post(&message->semaphore);
      break;
    case KILL:
      sem_post(&message->semaphore);
      break;
    default:
      flowstat_perror("bad message type");
      break;
    }
  }
  g_async_queue_unref(info->packet_queue);
  g_hash_table_destroy(peer_hshtbl);
}
