#include "hashtab.h"
#include "list.h"

#include <stdlib.h>
#include <string.h>

typedef struct hashtab_t {
	int size;
	list_t **table;
} hashtab_t;

static unsigned int _strhash(char *str, size_t size);

extern hashtab_t *hashtab_new(int size) {
	hashtab_t *ht = (hashtab_t*)malloc(sizeof(hashtab_t));
	ht->size = size;
	ht->table = (list_t**)malloc(sizeof(list_t*)*size);
	for (int i = 0; i < size; ++i) {
		ht->table[i] = list_new();
	}
	return ht;
}

extern void hashtab_free(hashtab_t *ht) {
	for (int i = 0; i < ht->size; ++i) {
		list_free(ht->table[i]);
	}
	free(ht->table);
	free(ht);
}

extern void *hashtab_select(hashtab_t *ht, char *key) {
	unsigned int hash = _strhash(key, ht->size);
	size_t lenkey = strlen(key)+1;
	char *val;
	for (int i = 0; (val = list_select(ht->table[hash], i)) != NULL; ++i) {
		if (strcmp(val, key) == 0) {
			return (void*)val + lenkey;
		}
	}
	return NULL;
}

extern int hashtab_insert(hashtab_t *ht, char *key, void *elem, int size) {
	unsigned int hash = _strhash(key, ht->size);
	size_t lenkey = strlen(key)+1;
	char *val;
	int rc, i;
	for (i = 0; (val = list_select(ht->table[hash], i)) != NULL; ++i) {
		if (strcmp(val, key) == 0) {
			break;
		}
	}
	val = (char*)malloc(size+lenkey);
	memcpy(val, key, lenkey);
	memcpy(val+lenkey, elem, size);
	rc = list_insert(ht->table[hash], i, val, size+lenkey);
	free(val);
	return rc;
}

extern int hashtab_delete(hashtab_t *ht, char *key) {
	unsigned int hash = _strhash(key, ht->size);
	char *val;
	for (int i = 0; (val = list_select(ht->table[hash], i)) != NULL; ++i) {
		if (strcmp(val, key) == 0) {
			return list_delete(ht->table[hash], i);
		}
	}
	return -1;
}

static unsigned int _strhash(char *str, size_t size) {
    unsigned int hashval;
    for (hashval = 0; *str != '\0'; ++str) {
        hashval = *str + 31 * hashval;
    }
    return hashval % size;
}
