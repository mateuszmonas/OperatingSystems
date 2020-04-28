CC = gcc -Wall

compile: clean
	make client
	make server

client:
	$(CC) client.c -o out/client -lrt

server:
	$(CC) server.c -o out/server -lrt

clean:
	rm -rf out/
	mkdir out/
