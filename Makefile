CC = gcc
ARG = -g3
OBJ = mem.o

test: test.c mem.h $(OBJ)
	$(CC) $(ARG) -o test test.c $(OBJ)

mem.o: mem.c
	$(CC) $(ARG) -c mem.c

.PHONY: clean
clean:
	rm -rf *.o test
