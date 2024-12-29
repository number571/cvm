#ifndef EXTCLIB_TYPE_LIST_H_
#define EXTCLIB_TYPE_LIST_H_

typedef struct list_t list_t;

extern list_t *list_new(void);
extern void list_free(list_t *ls);
extern int list_size(list_t *ls);

extern int list_find(list_t *ls, void *elem, int size);
extern void *list_select(list_t *ls, int index);
extern int list_insert(list_t *ls, int index, void *elem, int size);
extern int list_delete(list_t *ls, int index);

#endif /* EXTCLIB_TYPE_LIST_H_ */
