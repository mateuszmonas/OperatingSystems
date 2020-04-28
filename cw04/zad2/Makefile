CC = gcc -Wall

compile: clean
	make main

main:
	$(CC) exec_helper.c -o out/exec_helper
	$(CC) main.c -o out/main

clean:
	rm -rf out/
	mkdir out/

test: compile
	./out/main ignore
	./out/main handler
	./out/main mask
	./out/main pending
