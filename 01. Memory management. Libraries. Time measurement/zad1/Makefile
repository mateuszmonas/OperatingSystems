CC = gcc -Wall
OPT = O0

static: clean
	$(CC) -c mylib.c -o out/mylib.o -$(OPT)
	ar crs out/libmylib.a out/mylib.o

shared: clean
	$(CC) -fPIC -shared mylib.c -o out/libmylib.so -$(OPT)

clean:
	rm -rf out/
	mkdir out/
