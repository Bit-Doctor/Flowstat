##
## Makefile for flowstat in /home/jonathan/flowstat
## 
## Made by Jonathan Machado
## Login   <jonathan.machado@epitech.net>
## 
## Started on  Fri Sep  2 11:36:43 2011 Jonathan Machado
## Last update Tue Dec 13 16:14:33 2011 Jonathan Machado
##

NAME=		flowstat
USAGE=		
LIB_DIR=	liblist/
LIB=		$(LIB_DIR)liblist.a
INCLUDES_DIR=	includes/
INCLUDES=	$(INCLUDES_DIR)flowstat.h
SRCS_DIR=	srcs/
SRCS=		$(wildcard $(SRCS_DIR)*.c)
LFLAGS=		-pthread -lnetfilter_log -lnfnetlink -lrt -lglib-2.0 -lgthread-2.0
IFLAGS=		-I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I$(LIB_DIR) -I$(INCLUDES_DIR) 
CFLAGS=		-W -Wall $(IFLAGS) $(LFLAGS) -O3

OBJS_DIR=       $(SRCS_DIR).objs/
OBJS=		$(SRCS:%.c=%.o)
OBJS:=		$(notdir $(OBJS))
OBJS:=		$(addprefix $(OBJS_DIR), $(OBJS))

all:		lib $(NAME)

$(NAME): 	$(OBJS)
		@$(CC) $(LIB) -o $@ $^ $(CFLAGS)
		@echo -e "[\033[32mOK\033[0m] Files linked in" $(NAME)".\n"    		  	\
			"\nCompilation \033[32mdone\033[0m using        :" $(CFLAGS)	 	\
			"\n\nUsage : ./"$(NAME) $(USAGE)"\n"

lib:
		@make -C $(LIB_DIR)

lib_clean:
		@make -C $(LIB_DIR) clean

re: 		clean lib $(NAME)

clean:		lib_clean
		@rm -rf $(OBJS_DIR)*.o
		@rm -f $(NAME)
		@echo -e "[\033[32mOK\033[0m] project directory cleaned."

.PHONY:		all re clean

$(OBJS_DIR)%.o: $(SRCS_DIR)%.c $(INCLUDES)
		@$(CC) -c $(CFLAGS) -o $@ $<
		@echo -e "[\033[32mOK\033[0m] Compiled" $<

