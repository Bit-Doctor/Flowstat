/*
** verified_fct.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep  7 14:30:08 2011 Jonathan Machado
** Last update Fri Sep 16 10:30:15 2011 Jonathan Machado
*/

#include <syslog.h>
#include "flowstat.h"
#include "libipulog/libipulog.h"

void			flowstat_perror(char *str)
{
  if (str && !errno)
    syslog(LOG_ERR, "%s\n", str);
  else if (!ipulog_errno && !errno)
    syslog(LOG_ERR, "%s\n", "ERROR");
  else if (ipulog_errno)
    syslog(LOG_ERR, "%s\n", ipulog_strerror(ipulog_errno));
  else if (str && errno)
    syslog(LOG_ERR, "%s: %m\n", str);
  else if (errno)
    syslog(LOG_ERR, "%m\n");
#ifdef DEBUG
  if (str)
    fprintf(stderror, "%s\n", str);
  else if (!ipulog_errno && !errno)
    fprintf(stderror, "%s\n", "ERROR");
  else if (ipulog_errno)
    fprintf(stderror, "%s\n", ipulog_strerror(ipulog_errno));
  else if (errno)
    fprintf(stderror, "%s\n", strerror(errno));
#endif
}

struct ipulog_handle    *verified_ipulog_create_handle(u_int32_t group_mask, u_int32_t rcvbufsize)
{
  struct ipulog_handle  *handler;

  handler = ipulog_create_handle(group_mask, rcvbufsize);
  if (!handler)
    {
      flowstat_perror(NULL);
      exit(EXIT_FAILURE);
    }
  return (handler);
}

void			*xmalloc(int size)
{
  void			*ret;

  ret = malloc(size);
  if (ret == NULL)
    {
      flowstat_perror("malloc");
      exit(EXIT_FAILURE);
    }
  return (ret);
}
