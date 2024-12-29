#include "stack.h"

#include <stdlib.h>
#include <string.h>

typedef struct stack_t {
	int size;
	int valsize;
	int currpos;
	char *buffer;
} stack_t;

extern stack_t *stack_new(int size, int valsize) {
	stack_t *st = (stack_t*)malloc(sizeof(stack_t));
	st->size = size;
	st->valsize = valsize;
	st->currpos = 0;
	st->buffer = (char*)malloc(size*valsize);
	return st;
}

extern void stack_free(stack_t *st) {
	free(st->buffer);
	free(st);
}

extern int stack_size(stack_t *st) {
	return st->currpos;
}

extern int stack_push(stack_t *st, void *elem) {
	if (st->currpos == st->size) {
		return 1;
	}
	memcpy(st->buffer + st->currpos * st->valsize, elem, st->valsize);
	st->currpos += 1;
	return 0;
}

extern void *stack_pop(stack_t *st) {
	if (st->currpos == 0) {
		return NULL;
	}
	st->currpos -= 1;
	return (void*)st->buffer + st->currpos * st->valsize;
}

extern int stack_set(stack_t *st, int index, void *elem) {
	if (index < 0 || index >= st->size) {
		return 1;
	}
	memcpy(st->buffer + index * st->valsize, elem, st->valsize);
	return 0;
}

extern void *stack_get(stack_t *st, int index) {
	if (index < 0 || index >= st->size) {
		return NULL;
	}
	return (void*)st->buffer + index * st->valsize;
}
