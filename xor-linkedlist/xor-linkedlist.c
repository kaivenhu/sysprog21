#include <stdlib.h>

#include "xor-linkedlist.h"

void insert_node(list **l, int d)
{
    list *tmp = malloc(sizeof(list));
    tmp->data = d;

    if (!(*l)) {
        tmp->addr = NULL;
    } else {
        (*l)->addr = XOR(tmp, (*l)->addr);
        tmp->addr = *l;
    }
    *l = tmp;
}

void delete_list(list *l)
{
    while (l) {
        list *next = l->addr;
        if (next)
            next->addr = XOR(next->addr, l);
        free(l);
        l = next;
    }
}

static inline list *to_half(list *head)
{
    list *prev = NULL;
    list *fast = head;
    list *fast_next = head->addr;
    if (!head)
        return head;

    while (fast && fast_next) {
        list *tmp = head;
        head = XOR(head->addr, prev);
        prev = tmp;
        fast = XOR(fast_next->addr, fast);
        if (fast)
            fast_next = XOR(fast->addr, fast_next);
    }
    prev->addr = XOR(prev->addr, head);
    head->addr = XOR(head->addr, prev);
    return head;
}

list *sort(list *start)
{
    if (!start || !start->addr)
        return start;

    list *left = start, *right = to_half(start);

    left = sort(left);
    right = sort(right);

    for (list *merge = NULL; left || right;) {
        if (!right || (left && left->data < right->data)) {
            list *next = left->addr;
            if (next)
                next->addr = XOR(left, next->addr);

            if (!merge) {
                start = merge = left;
                merge->addr = NULL;
            } else {
                merge->addr = XOR(merge->addr, left);
                left->addr = merge;
                merge = left;
            }
            left = next;
        } else {
            list *next = right->addr;
            if (next)
                next->addr = XOR(right, next->addr);

            if (!merge) {
                start = merge = right;
                merge->addr = NULL;
            } else {
                merge->addr = XOR(merge->addr, right);
                right->addr = merge;
                merge = right;
            }
            right = next;
        }
    }

    return start;
}
