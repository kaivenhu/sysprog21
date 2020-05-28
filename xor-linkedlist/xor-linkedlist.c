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

static inline list *to_half(list *head, int *size)
{
    list *prev = NULL;
    list *fast = head;
    list *fast_next = head->addr;
    if (!head)
        return head;
    *size = 0;

    while (fast && fast_next) {
        ++(*size);
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

list *insertion_sort(list *head)
{
    list *sorted = head, *sorted_prev = NULL;
    if (!sorted || !sorted->addr)
        return sorted;

    while (sorted) {
        list *target = XOR(sorted->addr, sorted_prev);
        list *pos = sorted;
        list *prev = target;
        while (pos && target && pos->data > target->data) {
            list *tmp = pos;
            pos = XOR(pos->addr, prev);
            prev = tmp;
        }
        if (sorted != pos) {
            list *next = XOR(target->addr, sorted);
            sorted->addr = XOR(XOR(sorted->addr, target), next);
            if (!pos) {
                head = target;
                target->addr = prev;
                prev->addr = XOR(prev->addr, target);
            } else {
                pos->addr = XOR(XOR(pos->addr, prev), target);
                prev->addr = XOR(XOR(prev->addr, pos), target);
                target->addr = XOR(pos, prev);
            }
            if (!next)
                break;
            next->addr = XOR(XOR(next->addr, target), sorted);
            sorted_prev = XOR(sorted->addr, next);
        } else {
            pos = sorted;
            sorted = XOR(sorted->addr, sorted_prev);
            sorted_prev = pos;
        }
    }
    return head;
}

list *__merge_sort(list *start, int min_split_size)
{
    int size = 0;
    list *left = start, *right = NULL;

    if (!start || !start->addr)
        return start;

    if (!min_split_size) {
        return insertion_sort(start);
    } else {
        right = to_half(start, &size);
        if (size <= min_split_size)
            min_split_size = 0;
    }

    left = __merge_sort(left, min_split_size);
    right = __merge_sort(right, min_split_size);

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
