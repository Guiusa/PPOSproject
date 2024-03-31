FLAGS = -Wall
CC = gcc
PROG = queue

all: $(PROG)

$(PROG): teste.o queue.o
	$(CC) $(FLAGS) -o $(PROG) teste.o queue.o

teste.o: teste.c
	$(CC) $(FLAGS) -c teste.c -o teste.o

queue.o: queue.c
	$(CC) $(FLAGS) -c queue.c -o queue.o

purge:
	rm -rf *.o $(PROG)
