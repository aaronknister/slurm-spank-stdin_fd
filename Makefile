stdio_fds.so: stdio_fds.c
	$(CC) -shared -fPIC -o $@ $? $(CFLAGS)
