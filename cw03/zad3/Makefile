CC = gcc -Wall

compile: clean
	make matrix
	make helper

matrix:
	$(CC) main.c -o out/main

helper:
	make -C ../zad2

clean:
	rm -rf out/
	mkdir out/

test: compile
	./../zad2/out/helper generate ../test_files/matrices 10 10 20
	./out/main ../test_files/matrices 3 5 shared 10 10
	./../zad2/out/helper check ../test_files/matrices
	./out/main ../test_files/matrices 3 5 separate 10 10
	./../zad2/out/helper check ../test_files/matrices
