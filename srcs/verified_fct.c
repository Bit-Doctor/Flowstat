/*
** verified_fct.c for flowstat in /home/jonathan/Projets/flowstat
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Wed Sep  7 14:30:08 2011 Jonathan Machado
** Last update Wed Sep  7 14:38:41 2011 Jonathan Machado
*/

#include "flowstat.h"
#include "libipulog/libipulog.h"

struct ipulog_handle	*verified_ipulog_create_handle(u_int32_t gmask, u_int32_t rcvbufsize)
{
  struct ipulog_handle	*h;

  h = ipulog_create_handle(gmask, rcvbufsize);
  if (!h)
    {
      ipulog_perror(NULL);
      exit(EXIT_FAILURE);
    }
  return (h);
}

void			*xmalloc(int size)
{
  void			*ret;

  ret = malloc(size);
  if (ret == NULL)
    {
      ipulog_perror(NULL);
      exit(EXIT_FAILURE);
    }
  return (ret);
}
