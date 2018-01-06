CC = gcc -g3 -Wall
_SRC = main input curves hash metrics initialization assignment update
SRC = $(patsubst %, %.c, $(_SRC))
OBJ = $(patsubst %, %.o, $(_SRC))
HDRS = hash.h curves.h metrics.h initialization.h assignment.h update.h
OUT = lsh
	
all: $(OBJ) $(HDRS)
	$(CC) -o $(OUT) $(OBJ) -lm

%.o: %.c $(HDRS)
	$(CC) -o $@ -c $<
	
clean:
	rm -rf *.o