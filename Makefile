CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb -Wno-char-subscripts -Wno-type-limits
LDFLAGS=-flto -lcurses -lm -msse2 -mavx2 -ggdb -lid3tag

OBJ=src/main.o


all: $(OBJ) | bin
	$(CC) $(OBJ) $(CFLAGS) $(LDFLAGS) -o bin/cleo

$(OBJ):%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $<

bin:
	mkdir -p $@
