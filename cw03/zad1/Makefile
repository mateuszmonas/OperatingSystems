CC = gcc -Wall

compile: clean
	$(CC) main.c -o out/main


clean:
	rm -rf out/
	mkdir out/

test: compile
	./out/main ../test_files