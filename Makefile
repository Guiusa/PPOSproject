FLAGS = -Wall
CC = gcc
PROG = main.c

all:
	$(CC) $(FLAGS) queue.c ppos_core.c $(PROG) -o scheduler

debug:
	$(CC) $(FLAGS) -DDEBUG queue.c ppos_core.c $(PROG) -o scheduler

purge:
	rm -rf scheduler
