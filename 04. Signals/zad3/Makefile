CC = gcc -Wall

compile: clean
	make main

main:
	$(CC) main.c -o out/main

clean:
	rm -rf out/
	mkdir out/

test: compile
	./out/main sigalarm