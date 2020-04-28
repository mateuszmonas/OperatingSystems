CC = gcc -Wall

compile: clean
	make manager
	make worker1
	make worker2
	make worker3

manager:
	$(CC) manager.c -o out/manager

worker1:
	$(CC) worker1.c -o out/worker1

worker2:
	$(CC) worker2.c -o out/worker2

worker3:
	$(CC) worker3.c -o out/worker3

clean:
	rm -rf out/
	mkdir out/

test: compile
	./out/manager 1 1 1
