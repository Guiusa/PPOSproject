FLAGS = -Wall
CC = gcc
PROG = main.c
OUT = ppos

all:
	$(CC) $(FLAGS) queue.c ppos_core.c $(PROG) -o $(OUT) -lm

debug:
	$(CC) $(FLAGS) -DDEBUG queue.c ppos_core.c $(PROG) -o $(OUT) -lm

purge:
	rm -rf $(OUT)
