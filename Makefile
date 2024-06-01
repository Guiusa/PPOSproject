FLAGS = -Wall
CC = gcc
PROG = main.c
OUT = ppos

all:
	$(CC) $(FLAGS) queue.c hard_disk.c ppos_disk.c ppos_core.c $(PROG) -o $(OUT) -lm -lrt

debug:
	$(CC) $(FLAGS) -DDEBUG queue.c hard_disk.c ppos_disk.c ppos_core.c $(PROG) -o $(OUT) -lm -lrt

purge:
	rm -rf $(OUT)
