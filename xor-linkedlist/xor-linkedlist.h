#ifndef XOR_LINKEDLIST_H_
#define XOR_LINKEDLIST_H_
#include <stdint.h>

typedef struct __list list;
struct __list {
    int data;
    struct __list *addr;
};

#define XOR(a, b) ((list *) ((uintptr_t)(a) ^ (uintptr_t)(b)))

void insert_node(list **l, int d);
void delete_list(list *l);

#define sort(x) merge_sort(x)


list *insertion_sort(list *head);

#define merge_sort(x) __merge_sort(x, 1)
list *__merge_sort(list *start, int min_merge_size);

#endif /* XOR_LINKEDLIST_H_ */
