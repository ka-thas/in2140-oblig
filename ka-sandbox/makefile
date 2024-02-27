CFLAGS = -g -Wextra -Wall -std=gnu11
BIN = struct test

all: $(BIN)
	
struct: struct.c
	gcc $(CFLAGS) struct.c -o struct
test: test.c
	gcc $(CFLAGS) test.c -o test

