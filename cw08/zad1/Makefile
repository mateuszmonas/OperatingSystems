CC = gcc -Wall

compile: clean
	$(CC) main.c -o out/main -lpthread -lm

clean:
	rm -rf out/
	mkdir out/

test: compile
	./out/main 1 interval fractal_tree.ascii.pgm output.txt > Times.txt
	./out/main 2 interval fractal_tree.ascii.pgm output.txt >> Times.txt
	./out/main 4 interval fractal_tree.ascii.pgm output.txt >> Times.txt
	./out/main 8 interval fractal_tree.ascii.pgm output.txt >> Times.txt
	./out/main 1 block fractal_tree.ascii.pgm output.txt >> Times.txt
	./out/main 2 block fractal_tree.ascii.pgm output.txt >> Times.txt
	./out/main 4 block fractal_tree.ascii.pgm output.txt >> Times.txt
	./out/main 8 block fractal_tree.ascii.pgm output.txt >> Times.txt
	./out/main 1 sign fractal_tree.ascii.pgm output.txt >> Times.txt
	./out/main 2 sign fractal_tree.ascii.pgm output.txt >> Times.txt
	./out/main 4 sign fractal_tree.ascii.pgm output.txt >> Times.txt
	./out/main 8 sign fractal_tree.ascii.pgm output.txt >> Times.txt
