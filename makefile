CC = gcc -g3 -Wall
OUT = lsh

all:
	$(CC) main.c dtw.c dfd.c euclid.c input.c curves.c hash.c -lm -o $(OUT)

main.o: main.c 
	$(CC) -c main.c
	
clean:
	rm -rf *.o $(OUT)