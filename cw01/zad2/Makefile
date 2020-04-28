CC = gcc -Wall
LIB_PATH = ../zad1

static: clean
	make -C $(LIB_PATH) static
	$(CC) main.c -L$(LIB_PATH)/out -lmylib -o out/main $(OPT)

shared: clean
	make -C $(LIB_PATH) shared
	$(CC) main.c -L$(LIB_PATH)/out -Wl,-rpath=$(LIB_PATH)/out -lmylib -o out/main $(OPT)

clean:
	rm -rf out/
	mkdir out/
