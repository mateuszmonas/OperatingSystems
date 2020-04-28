CC = gcc -Wall

compile: clean
	make matrix
	make helper

matrix:
	$(CC) main.c -o out/main

helper:
	$(CC) helper.c -o out/helper

clean:
	rm -rf out/
	mkdir out/

test: compile
	./out/helper generate ../test_files/matrices 10 10 20
	./out/main ../test_files/matrices 3 5 shared
	./out/helper check ../test_files/matrices
	./out/main ../test_files/matrices 3 5 separate
	./out/helper check ../test_files/matrices
