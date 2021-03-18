CC=gcc
CFLAGS=-Wall -std=c99

FILES=cvm.c readvm.c extclib/type/stack.c extclib/type/hashtab.c extclib/type/list.c 

.PHONY: default install build run clean
default: build run 
install:
	git clone https://github.com/number571/extclib.git || true
	make -C extclib/
build: $(FILES)
	$(CC) -o cvm $(CFLAGS) $(FILES)
run: cvm main.vms
	./cvm build main.vms -o main.vme
	./cvm run main.vme
clean:
	rm -f cvm main.vme
