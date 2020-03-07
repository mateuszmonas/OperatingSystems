CC = gcc -Wall

static: clean
	$(CC) -c mylib.c -o out/mylib.o
	ar crs out/libmylib.a out/mylib.o

shared: clean
	$(CC) -fPIC -shared out/mylib.c -o out/libmylib.so

clean:
	rm -rf out/
	mkdir out/