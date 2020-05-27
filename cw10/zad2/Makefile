CC = gcc -Wall

compile: clean
	make client
	make server

client:
	$(CC) client.c -o out/client -lpthread

server:
	$(CC) server.c -o out/server -lpthread

clean:
	rm -rf out/
	mkdir out/
