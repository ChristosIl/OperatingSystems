CC = gcc
CFLAGS = -Wall

#parent is named gates to follow ./gates execution as per the example

all: gates child

.PHONY: all clean

child: child.o
	gcc child.o -o child
child.o: child.c
	gcc child.c -c -o child.o

gates: gates.o
	gcc gates.o -o gates
gates.o: gates.c
	gcc gates.c -c -o gates.o

clean:
	rm -f gates child

