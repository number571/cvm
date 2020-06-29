CC=gcc
CFLAGS=-Wall -std=c99

FILES=main.c readsm.c extclib/extclib.o

.PHONY: default build run clean
default: build run

build: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o main -lgmp
run: main
	./main main.sm
clean:
	rm -f main
