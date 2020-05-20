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
list *sort(list *start);

#endif /* XOR_LINKEDLIST_H_ */
