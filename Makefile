CC=gcc
CFLAGS=-Wall -std=c99

FILES=main.c readsm.c extclib/extclib.o

.PHONY: default build run clean
default: build run

build: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o main
run: main
	./main
clean:
	rm -f main main.vme
