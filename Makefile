CC=gcc
CFLAGS=

all: main.o
	./main.o ~/Downloads/xf1-load bulk

main.o: main.c Makefile
	$(CC) $(CFLAGS) main.c -o main.o
