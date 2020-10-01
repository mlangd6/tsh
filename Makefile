CC=gcc
CFLAGS=-g -Wall
EXEC=tsh

OBJS = src/main.o src/tar.o src/errors.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)


src/main.o: src/main.c

src/tar.o: src/tar.h src/tar.c

src/errors.o: src/errors.h src/errors.h

clean:
	rm -f src/*~ $(EXEC) $(OBJS)
