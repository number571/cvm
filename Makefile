CC=gcc
CFLAGS=-Wall -std=c99

FILES=cvm.c readvm.c extclib/extclib.o

.PHONY: default build run clean
default: build run 

build: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o cvm
run: cvm 
	./cvm build main.vms -o main.vme
	./cvm run main.vme
clean:
	rm -f cvm main.vme
