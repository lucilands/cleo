CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb
LDFLAGS=-flto -lcurses -lm -msse2 -mavx2 -ggdb

OBJ=src/main.o


all: $(OBJ) | bin
	$(CC) $(OBJ) $(CFLAGS) $(LDFLAGS) -o bin/cleo

$(OBJ):%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $<

bin:
	mkdir -p $@
