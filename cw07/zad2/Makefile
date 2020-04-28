CC = gcc -Wall

compile: clean
	make manager
	make worker1
	make worker2
	make worker3

manager:
	$(CC) manager.c -o out/manager -lrt -lpthread

worker1:
	$(CC) worker1.c -o out/worker1 -lrt -lpthread

worker2:
	$(CC) worker2.c -o out/worker2 -lrt -lpthread

worker3:
	$(CC) worker3.c -o out/worker3 -lrt -lpthread

clean:
	rm -rf out/
	mkdir out/

test: compile
	./out/manager 1 1 1
