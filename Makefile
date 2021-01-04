CC=gcc
CFLAGS=-Wall -std=c99

FILES=cvm.c readvm.c extclib/extclib.o

.PHONY: default install build run clean
default: build run 
install:
	git clone https://github.com/number571/extclib.git || true
	make -C extclib/
build: $(FILES)
	$(CC) -o cvm $(CFLAGS) $(FILES) -lcrypto
run: cvm 
	./cvm build main.vms -o main.vme
	./cvm run main.vme
clean:
	rm -f cvm main.vme
