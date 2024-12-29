CC=gcc
CFLAGS=-Wall -std=c99

FILES=cvm.c cvmkernel.c typeslib/stack.c typeslib/hashtab.c typeslib/list.c 

.PHONY: default build run clean
default: build run 

build: $(FILES)
	$(CC) -o cvm $(CFLAGS) $(FILES)
run:
	./cvm build main.asm -o main.bcode
	./cvm run main.bcode
clean:
	rm -f cvm main.asm main.bcode
