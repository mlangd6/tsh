CC=gcc
CFLAGS=-g -Wall
EXEC=tsh

all: $(EXEC)

$(EXEC): src/main.o
	$(CC) -o $@ $^


src/main.o: src/main.c

src/tar.o: src/tar.h src/tar.c


clean:
	rm -f src/*.o src/*~ $(EXEC)
