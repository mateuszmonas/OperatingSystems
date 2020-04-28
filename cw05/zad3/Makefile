CC = gcc -Wall

compile: clean
	make producent
	make konsument
	make helper

producent:
	$(CC) producent.c -o out/producent

helper:
	$(CC) helper.c -o out/helper

konsument:
	$(CC) konsument.c -o out/konsument

clean:
	rm -rf out/
	mkdir out/

test_helper: compile
	./out/helper

test: compile
	rm -r fifo
	mkfifo fifo
	./out/producent fifo in1.txt 25 &
	./out/producent fifo in2.txt 25 &
	./out/konsument fifo out.txt 100
