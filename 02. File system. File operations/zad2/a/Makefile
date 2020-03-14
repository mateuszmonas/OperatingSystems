CC = gcc -Wall

compile: clean
	$(CC) main.c -o out/main


clean:
	rm -rf out/
	mkdir out/

test:
	make compile
	./out/main ../../test_files
	./out/main ../../test_files -maxdepth 1
	./out/main ../../test_files -maxdepth 1 -atime 1