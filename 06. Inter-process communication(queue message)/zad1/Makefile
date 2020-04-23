CC = gcc -Wall

compile: clean
	make client
	make server

client:
	$(CC) client.c -o out/client

server:
	$(CC) server.c -o out/server

clean:
	rm -rf out/
	mkdir out/
