CC = gcc -Wall

compile: clean
	$(CC) main.c -o out/main -lpthread

clean:
	rm -rf out/
	mkdir out/

test: compile
	./out/main 2 10
