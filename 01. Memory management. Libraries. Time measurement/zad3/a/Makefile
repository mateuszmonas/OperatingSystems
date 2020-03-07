CC = gcc -Wall

static: clean
	$(CC) -c ../../zad1/mylib.c -o out/mylib.o
	ar crs out/libmylib.a out/mylib.o
	$(CC) main.c -Lout -lmylib -o out/main

shared: clean
	$(CC) -fPIC -shared ../../zad1/mylib.c -o out/libmylib.so
	$(CC) main.c -Lout -Wl,-rpath=out -lmylib -o out/main

dynamic: clean
	$(CC) -fPIC -shared ../../zad1/mylib.c -o out/libmylib.so
	$(CC) main_dynamic.c -Lout -Wl,-rpath=out -lmylib -ldl -o out/main

clean:
	rm -rf out/
	mkdir out/

test:
	./out/main create_table 2 compare_pairs ../../test_files/a.txt:../../test_files/b.txt ../../test_files/c.txt:../../test_files/d.txt