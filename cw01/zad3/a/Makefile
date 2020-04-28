CC = gcc -Wall
LIB_PATH = ../../zad1

static: clean
	make -C $(LIB_PATH) static
	$(CC) main.c -L$(LIB_PATH)/out -lmylib -o out/main

shared: clean
	make -C $(LIB_PATH) shared
	$(CC) main.c -L$(LIB_PATH)/out -Wl,-rpath=$(LIB_PATH)/out -lmylib -o out/main

dynamic: clean
	make -C $(LIB_PATH) shared
	$(CC) main_dynamic.c -L$(LIB_PATH)/out -Wl,-rpath=$(LIB_PATH)/out -lmylib -ldl -o out/main

clean:
	rm -rf out/
	mkdir out/
