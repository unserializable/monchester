CC = $(shell command -v gcc-8 || command -v gcc-7 || command -v gcc)
GCC_GT7 = $(shell (test $$($(CC) -dumpversion | cut -d. -f1) -ge 7 && echo true)  || echo false)
ifeq "true" "$(GCC_GT7)"
GCC7FLAGS = -Walloc-zero -Wformat-overflow
endif

OBJS = globals.o iomain.o xtdlib.o move.o brdlist.o compmove.o fen.o

ARFLAGS = rcsD
CFLAGS = -DDEBUG=1 -DDEBUG_EVAL=1 -g -Wformat -Wstrict-prototypes -Wall -fsanitize=address $(GCC7FLAGS)
CFLAGS += -DMONCHESTER_VERSION="1.0"
CFLAGS += $(EXTFLAGS)

monchester: main.c features.h xtdlib.h types.h fen.h globals.h iomain.h move.h brdlist.h compmove.h $(OBJS)
	$(CC) $(CFLAGS) main.c -o monchester $(OBJS)

xtdlib.o: xtdlib.c xtdlib.h features.h types.h globals.h brdlist.h
	$(CC) $(CFLAGS) -c xtdlib.c

move.o: move.c move.h features.h types.h bishop.h knight.h rook.h queen.h king.h globals.h xtdlib.h iomain.h
	$(CC) $(CFLAGS) -c move.c

iomain.o: iomain.c iomain.h features.h types.h xtdlib.h fen.h globals.h brdlist.h
	$(CC) $(CFLAGS) -c iomain.c

globals.o: globals.c features.h types.h
	$(CC) $(CFLAGS) -c globals.c

brdlist.o: brdlist.c brdlist.h features.h types.h globals.h xtdlib.h move.h fen.h iomain.h
	$(CC) $(CFLAGS) -c brdlist.c

compmove.o: compmove.c compmove.h features.h types.h iomain.h move.h globals.h xtdlib.h
	$(CC) $(CFLAGS) -c compmove.c

fen.o: fen.c move.h features.h types.h fen.h xtdlib.h globals.h
	$(CC) $(CFLAGS) -c fen.c

release: CFLAGS=-DNDEBUG -Os -Wformat -Wstrict-prototypes -Wall $(GCC7FLAGS)
release: CFLAGS += -DMONCHESTER_VERSION="1.0"
release: CFLAGS += $(EXTFLAGS)
release: monchester

clean:
	rm -f monchester *.o core
