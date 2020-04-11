CC = gcc -Wall

compile: clean
	make producent
	make konsument

producent:
	$(CC) producent.c -o out/producent

konsument:
	$(CC) konsument.c -o out/konsument

clean:
	rm -rf out/
	mkdir out/

test: compile
	rm -r fifo
	mkfifo fifo
	./out/producent fifo in1.txt 25 &
	./out/producent fifo in2.txt 25 &
	./out/konsument fifo out.txt 100
