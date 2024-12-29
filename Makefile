CC=gcc
CFLAGS=-Wall -std=c99

FILES=cvm.c cvmkernel.c typeslib/stack.c typeslib/hashtab.c typeslib/list.c 

.PHONY: default build run clean
default: build run 

build: $(FILES)
	$(CC) -o cvm $(CFLAGS) $(FILES)
run:
	./cvm build main.asm -o main.bcd
	./cvm run main.bcd
clean:
	rm -f cvm main.asm main.bcd
