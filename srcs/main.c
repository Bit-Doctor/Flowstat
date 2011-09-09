/*
** main.c for flowstat in /home/jonathan/flowstat/srcs
**
** Made by Jonathan Machado
** Login   <jonathan.machado@epitech.net>
**
** Started on  Fri Sep  2 12:00:57 2011 Jonathan Machado
** Last update Fri Sep  9 14:32:16 2011 Jonathan Machado
*/


#include "flowstat.h"
#include "cson_amalgamation_core.h"

cson_value		*rootV = NULL;
cson_object		*root = NULL;
cson_array		*flux = NULL;

void			create_new_json(void)
{
  char			*date;
  time_t		t;
  cson_value		*fluxV;

  time(&t);
  date = ctime(&t);
  date[24] = 0;
  rootV = cson_value_new_object();
  root = cson_value_get_object(rootV);
  fluxV = cson_value_new_array();
  flux = cson_value_get_array(fluxV);
  cson_object_set(root, "Log start", cson_value_new_string(date, strlen(date)));
  cson_object_set(root, "Log start time", cson_value_new_integer(t));
  cson_object_set(root, "Last save", cson_value_new_string("", 0));
  cson_object_set(root, "Last save time", cson_value_new_integer(0));
  cson_object_set(root, "IP", fluxV);
}

void			init_log_file(void)
{
  int			error;
  FILE			*file;

  file = fopen("./flowstat.log", "r+");
  if (file == NULL)
    create_new_json();
  else
    {
      error = cson_parse_FILE(&rootV, file, NULL, NULL);
      if (error != 0)
	{
	  printf("Error code %d (%s)!\n", error, cson_rc_string(error));
	  printf("During the next save the log file will be flushed\n");
	  create_new_json();
	}
      root = cson_value_get_object(rootV);
      flux = cson_value_get_array(cson_object_get(root, "IP"));
      fclose(file);
    }
}

void			save_json_in_file(void)
{

  char			*date;
  time_t		t;
  FILE			*file;
  cson_output_opt      	opt;

  time(&t);
  date = ctime(&t);
  date[24] = 0;
  file = fopen("./flowstat.log", "w");
  cson_object_set(root, "Last save", cson_value_new_string(date, strlen(date)));
  cson_object_set(root, "Last save time", cson_value_new_integer(t));
  opt = cson_output_opt_empty;
  opt.indentation = 0;
  cson_output_FILE(rootV, file, &opt);
  fclose(file);
}

void			read_and_analyze(struct ipulog_handle *h)
{
  int			len;
  time_t		last_save;
  unsigned char		*buffer;
  ulog_packet_msg_t	*ulog_packet;

  time(&last_save);
  buffer = xmalloc(BUFFER_SIZE * sizeof(*buffer));
  while ((len = ipulog_read(h, buffer, BUFFER_SIZE, 1)))
    {
      while ((ulog_packet = ipulog_get_packet(h, buffer, len)))
	packet_handler(ulog_packet);
      if (time(NULL) - 3 > last_save)
	{
	  save_json_in_file();
	  time(&last_save);
	}
    }
  free(buffer);
}

 #include <signal.h>
  struct ipulog_handle	*connection;

void	free_at_interupt(int signum)
{
  ipulog_destroy_handle(connection);
  cson_value_free(rootV);
  exit(0);
}

int			main(void)
{


  /* if (fork()) */
  /*   exit(0); */
  /* add rule for iptables with system or excve */

  signal(SIGINT, &free_at_interupt);
  init_log_file();
  connection = verified_ipulog_create_handle(ipulog_group2gmask(GROUP_NETLINK), 150000);
  read_and_analyze(connection);
   return (EXIT_SUCCESS);
}
