FLAGS = -Wall
CC = gcc
PROG = main.c

all:
	$(CC) $(FLAGS) queue.c ppos_core.c $(PROG) -o dispatcher

debug:
	$(CC) $(FLAGS) -DDEBUG queue.c ppos_core.c $(PROG) -o dispatcher

purge:
	rm -rf dispatcher
