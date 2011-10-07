
CFLAGS+=-Iinclude -I/usr/src/linux/include

all: libipulog.a

re: clean libipulog.a

libipulog.a: libipulog.o
	@$(LD) -i $< -o $@
	@echo -e "[\033[32mOK\033[0m] Lib linked"

libipulog.o: libipulog.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$(RM) libipulog.o libipulog.a
	@echo -e "[\033[32mOK\033[0m] Lib directory cleaned"