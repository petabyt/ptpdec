CC=gcc
CFLAGS=

all: main.o
	./main.o ~/Downloads/xf1-load bulk > bulk.txt

main.o: main.c Makefile
	$(CC) $(CFLAGS) main.c -o main.o

clean:
	$(RM) *.o *.txt

.PHONY: all clean
