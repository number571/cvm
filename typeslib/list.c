#include "list.h"

#include <stdlib.h>
#include <string.h>

typedef struct list_t {
	int size;
	void *elem;
	struct list_t *next;
} list_t;

extern list_t *list_new(void) {
	list_t *ls = (list_t*)malloc(sizeof(list_t));
	ls->size = 0;
	ls->elem = NULL;
	ls->next = NULL;
	return ls;
}

extern void list_free(list_t *ls) {
	list_t *next;
	while(ls != NULL) {
		next = ls->next;
		free(ls->elem);
		free(ls);
		ls = next;
	}
}

extern int list_find(list_t *ls, void *elem, int size) {
	for (int i = 0; ls->next != NULL; ++i) {
		ls = ls->next;
		if (ls->size == size && memcmp(ls->elem, elem, size) == 0) {
			return i;
		}
	}
	return -1;
}

extern void *list_get(list_t *ls, int index) {
	for (int i = 0; ls->next != NULL && i < index; ++i) {
		ls = ls->next;
	}
	if (ls->next == NULL) {
		return NULL;
	}
	return ls->next->elem;
}

extern int list_set(list_t *ls, int index, void *elem, int size) {
	list_t *root = ls;
	if (size <= 0) {
		return 1;
	}
	for (int i = 0; ls != NULL && i < index; ++i) {
		ls = ls->next;
	}
	if (ls == NULL) {
		return 2;
	}
	if (ls->next == NULL) {
		ls->next = list_new();
		ls->next->elem = (void*)malloc(size);
		root->size += 1;
	} else {
		ls->next->elem = (void*)realloc(ls->next->elem, size);
	}
	ls->next->size = size;
	memcpy(ls->next->elem, elem, size);
	return 0;
}

extern int list_del(list_t *ls, int index) {
	list_t *temp;
	list_t *root = ls;
	for (int i = 0; ls->next != NULL && i < index; ++i) {
		ls = ls->next;
	}
	if (ls->next == NULL) {
		return 1;
	}
	temp = ls->next;
	ls->next = ls->next->next;
	free(temp);
	root->size -= 1;
	return 0;
}

extern int list_size(list_t *ls) {
	return ls->size;
}
