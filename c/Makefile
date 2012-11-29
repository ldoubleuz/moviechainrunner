CC = gcc
CFLAGS = -Wall -std=gnu99 -g
LIBS = -lpthread

all:
	$(CC) $(CFLAGS) -O0 -o MCR MCR.c $(LIBS)

fast:
	$(CC) $(CFLAGS) -O3 -o MCR MCR.c $(LIBS)

debug:
	$(CC) $(CFLAGS) -O0 -D DEBUG -o MCR MCR.c $(LIBS)

clean:
	rm -f MCR.exe *~
