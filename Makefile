stdin_fd.so: stdin_fd.c
	$(CC) -shared -fPIC -o $@ $? $(CFLAGS)
