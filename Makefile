FLAGS = -Wall
CC = gcc
PROG = testeTasks

all:
	$(CC) $(FLAGS) ppos_core.c $(PROG).c -o $(PROG)

debug:
	$(CC) $(FLAGS) -DDEBUG ppos_core.c $(PROG).c -o $(PROG)

purge:
	rm -rf $(PROG)
