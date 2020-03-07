CC = gcc -Wall

static: clean
	$(CC) -c mylib.c -o mylib.o
	ar crs libmylib.a mylib.o
	$(CC) main.c -L. -lmylib -o main

shared: clean
	$(CC) -fPIC -shared mylib.c -o libmylib.so
	$(CC) main.c -L.  -Wl,-rpath=. -lmylib -o main

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main
	rm -f *.txt