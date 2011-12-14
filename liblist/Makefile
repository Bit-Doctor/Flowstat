##
## Makefile for libdatac in /home/jonathan/Projets/test/liblist
## 
## Made by Jonathan Machado
## Login   <jonathan.machado@epitech.net>
## 
## Started on  Tue Dec 13 14:01:02 2011 Jonathan Machado
## Last update Tue Dec 13 16:12:30 2011 Jonathan Machado
##

SRCS_DIR=	./
SRCS=           $(wildcard $(SRCS_DIR)*.c)
INCLUDES=	libdatac_list.h libdatac_link.h
OBJS_DIR=       $(SRCS_DIR).objs/
OBJS=           $(SRCS:%.c=%.o)
OBJS:=          $(notdir $(OBJS))
OBJS:=          $(addprefix $(OBJS_DIR), $(OBJS))
CFLAGS+=

all: liblist.a

re: clean liblist.a

liblist.a: $(OBJS)
		@$(LD) -r .objs/*.o -o $@
		@echo -e "[\033[32mOK\033[0m] Lib linked"

clean:
		@$(RM) liblist.o liblist.a .objs/*
		@echo -e "[\033[32mOK\033[0m] Lib directory cleaned"

.PHONY:		all re clean

$(OBJS_DIR)%.o: $(SRCS_DIR)%.c $(INCLUDES)
		@$(CC) -c $(CFLAGS) -o $@ $<
		@echo -e "[\033[32mOK\033[0m] Compiled" $<
