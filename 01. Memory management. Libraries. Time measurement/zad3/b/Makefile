CC = gcc -Wall
LIB_PATH = ../../zad1

static_o0: clean
	make -C $(LIB_PATH) static OPT=O0
	$(CC) main.c -L$(LIB_PATH)/out -lmylib -o out/main -O0

shared_o0: clean
	make -C $(LIB_PATH) shared OPT=O0
	$(CC) main.c -L$(LIB_PATH)/out -Wl,-rpath=$(LIB_PATH)/out -lmylib -o out/main -O0

dynamic_o0: clean
	make -C $(LIB_PATH) shared OPT=O0
	$(CC) main_dynamic.c -L$(LIB_PATH)/out -Wl,-rpath=$(LIB_PATH)/out -lmylib -ldl -o out/main -O0

static_o1: clean
	make -C $(LIB_PATH) static OPT=O1
	$(CC) main.c -L$(LIB_PATH)/out -lmylib -o out/main -O1

shared_o1: clean
	make -C $(LIB_PATH) shared OPT=O1
	$(CC) main.c -L$(LIB_PATH)/out -Wl,-rpath=$(LIB_PATH)/out -lmylib -o out/main -O1

dynamic_o1: clean
	make -C $(LIB_PATH) shared OPT=O1
	$(CC) main_dynamic.c -L$(LIB_PATH)/out -Wl,-rpath=$(LIB_PATH)/out -lmylib -ldl -o out/main -O1

static_o2: clean
	make -C $(LIB_PATH) static OPT=O2
	$(CC) main.c -L$(LIB_PATH)/out -lmylib -o out/main -O2

shared_o2: clean
	make -C $(LIB_PATH) shared OPT=O2
	$(CC) main.c -L$(LIB_PATH)/out -Wl,-rpath=$(LIB_PATH)/out -lmylib -o out/main -O2

dynamic_o2: clean
	make -C $(LIB_PATH) shared OPT=O2
	$(CC) main_dynamic.c -L$(LIB_PATH)/out -Wl,-rpath=$(LIB_PATH)/out -lmylib -ldl -o out/main -O2

clean:
	rm -rf out/
	mkdir out/
