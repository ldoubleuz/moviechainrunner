CC = gcc
CFLAGS = -Wall -O0 -std=gnu99 -g
LIBS = -lpthread

all:
	$(CC) $(CFLAGS) -o MCR MCR.c $(LIBS)

debug:
	$(CC) $(CFLAGS) -D DEBUG -o MCR MCR.c $(LIBS)

clean:
	rm -f MCR.exe *~
