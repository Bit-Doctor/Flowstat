#
## Makefile for flowstat in /home/jonathan/flowstat
## 
## Made by Jonathan Machado
## Login   <jonathan.machado@epitech.net>
## 
## Started on  Fri Sep  2 11:36:43 2011 Jonathan Machado
## Last update Fri Sep  9 09:46:07 2011 Jonathan Machado
##

NAME=		flowstat
USAGE=		
LIBS=		
INCLUDES_DIR=	includes/
INCLUDES=	$(INCLUDES_DIR)flowstat.h \
		$(INCLUDES_DIR)libipulog/libipulog.h \
		$(INCLUDES_DIR)netfilter_ipv4/ipt_ULOG.h
SRCS_DIR=	srcs/
SRCS=		$(wildcard $(SRCS_DIR)*.c)
CFLAGS=		-W -Wall -ansi -I$(INCLUDES_DIR) -I/usr/src/linux/include
OBJS_DIR=       $(SRCS_DIR).objs/
OBJS=		$(SRCS:%.c=%.o)
OBJS:=		$(notdir $(OBJS))
OBJS:=		$(addprefix $(OBJS_DIR), $(OBJS))


all: 		
	       cc srcs/*.c -Iincludes libs/libipulog/libipulog.a -o $(NAME) -Iincludes/cson libs/cson/*.c -g3

$(NAME): 	$(OBJS)
		@$(CC) -o $@ $^ $(CFLAGS)
		@echo -e "[\033[32mOK\033[0m] Files linked in" $(NAME)".\n"    		  	\
			"\nCompilation \033[32mdone\033[0m using        :" $(CFLAGS)	 	\
			"\n\nUsage : ./"$(NAME) $(USAGE)"\n"

re: 		clean $(NAME)

clean:
		@rm -rf $(OBJS_DIR)*.o
		@rm -f $(NAME)
		@echo -e "[\033[32mOK\033[0m] project directory cleaned."

.PHONY:		all re clean

$(OBJS_DIR)%.o: $(SRCS_DIR)%.c $(INCLUDES)
		@$(CC) -c $(CFLAGS) -I$(INCLUDES_DIR) -I/usr/src/linux/include -o $@ $<
		@echo -e "[\033[32mOK\033[0m] Compiled" $<
