CC = gcc -Wall

compile: clean
	make main

main:
	$(CC) main.c -o out/main -lrt

clean:
	rm -rf out/
	mkdir out/

test: compile
	./out/main sigchld
	./out/main sigqueue
	./out/main sigalarm