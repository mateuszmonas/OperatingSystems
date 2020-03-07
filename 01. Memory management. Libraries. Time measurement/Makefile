CC = gcc -Wall
OPT = -O0

static: clean
	$(CC) -c mylib.c -o mylib.o
	ar crs libmylib.a mylib.o
	$(CC) main.c -L. -lmylib -o main $(OPT)

shared: clean
	$(CC) -fPIC -shared mylib.c -o libmylib.so
	$(CC) main.c -L. -Wl,-rpath=. -lmylib -o main $(OPT)

dynamic: clean
	$(CC) -fPIC -shared mylib.c -o libmylib.so
	$(CC) main_dynamic.c -L. -Wl,-rpath=. -lmylib -ldl -o main_dynamic $(OPT)

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f main
	rm -f *.txt