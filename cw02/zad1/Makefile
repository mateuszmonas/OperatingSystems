CC = gcc -Wall

compile: clean
	$(CC) main.c -o out/main


clean:
	rm -rf out/
	mkdir out/

test:
	make compile
	./out/main generate ../test_files/dane 10000 8192

	./out/main copy ../test_files/dane ../test_files/dane_copy 81920 1 sys
	./out/main sort ../test_files/dane_copy 81920 1 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 81920 1 lib
	./out/main sort ../test_files/dane_copy 81920 1 lib

	./out/main copy ../test_files/dane ../test_files/dane_copy 40960 1 sys
	./out/main sort ../test_files/dane_copy 40960 1 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 40960 1 lib
	./out/main sort ../test_files/dane_copy 40960 1 lib

	./out/main copy ../test_files/dane ../test_files/dane_copy 89120 4 sys
	./out/main sort ../test_files/dane_copy 89120 4 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 89120 4 lib
	./out/main sort ../test_files/dane_copy 89120 4 lib

	./out/main copy ../test_files/dane ../test_files/dane_copy 40960 4 sys
	./out/main sort ../test_files/dane_copy 40960 4 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 40960 4 lib
	./out/main sort ../test_files/dane_copy 40960 4 lib

	./out/main copy ../test_files/dane ../test_files/dane_copy 80000 512 sys
	./out/main sort ../test_files/dane_copy 80000 512 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 80000 512 lib
	./out/main sort ../test_files/dane_copy 80000 512 lib

	./out/main copy ../test_files/dane ../test_files/dane_copy 40000 512 sys
	./out/main sort ../test_files/dane_copy 40000 512 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 40000 512 lib
	./out/main sort ../test_files/dane_copy 40000 512 lib

	./out/main copy ../test_files/dane ../test_files/dane_copy 40000 1024 sys
	./out/main sort ../test_files/dane_copy 40000 1024 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 40000 1024 lib
	./out/main sort ../test_files/dane_copy 40000 1024 lib

	./out/main copy ../test_files/dane ../test_files/dane_copy 20000 1024 sys
	./out/main sort ../test_files/dane_copy 20000 1024 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 20000 1024 lib
	./out/main sort ../test_files/dane_copy 20000 1024 lib

	./out/main copy ../test_files/dane ../test_files/dane_copy 20000 4096 sys
	./out/main sort ../test_files/dane_copy 20000 4096 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 20000 4096 lib
	./out/main sort ../test_files/dane_copy 20000 4096 lib

	./out/main copy ../test_files/dane ../test_files/dane_copy 10000 4096 sys
	./out/main sort ../test_files/dane_copy 10000 4096 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 10000 4096 lib
	./out/main sort ../test_files/dane_copy 10000 4096 lib

	./out/main copy ../test_files/dane ../test_files/dane_copy 1000 8192 sys
	./out/main sort ../test_files/dane_copy 1000 8192 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 1000 8192 lib
	./out/main sort ../test_files/dane_copy 1000 8192 lib

	./out/main copy ../test_files/dane ../test_files/dane_copy 500 8192 sys
	./out/main sort ../test_files/dane_copy 500 8192 sys
	./out/main copy ../test_files/dane ../test_files/dane_copy 500 8192 lib
	./out/main sort ../test_files/dane_copy 500 8192 lib