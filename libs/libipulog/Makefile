
CFLAGS+=-Iinclude -I/usr/src/linux/include

re: clean libipulog.a

libipulog.a: libipulog.o
	$(LD) -i $< -o $@

libipulog.o: libipulog.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) libipulog.o libipulog.a
