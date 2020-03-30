CC = gcc -Wall

compile: clean
	make sender
	make catcher

sender:
	$(CC) sender.c -o out/sender

catcher:
	$(CC) catcher.c -o out/catcher


clean:
	rm -rf out/
	mkdir out/

test: compile
	./out/main
