all:s c
s:server.c
	gcc -o s server.c ./02_内核列表/user_info.c ./01_线程池/thread_pool.c dir.c -pthread
c:shiyan.c
	gcc -o c shiyan.c ./02_内核列表/user_info.c  dir.c -pthread
clean:
	rm s c