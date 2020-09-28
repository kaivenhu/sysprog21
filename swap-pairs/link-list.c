#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct __node {
    int value;
    struct __node *next;
} node_t;

void add_entry(node_t **head, int new_value)
{
    node_t **indirect = head;

    node_t *new_node = malloc(sizeof(node_t));
    assert(new_node);
    new_node->value = new_value;
    new_node->next = NULL;

    while (*indirect)
        indirect = &(*indirect)->next;
    *indirect = new_node;
}

node_t *find_entry(node_t *head, int value)
{
    node_t *current = head;
    for (; current && current->value != value; current = current->next)
        /* interate */;
    return current;
}

void remove_entry(node_t **head, node_t *entry)
{
    node_t **indirect = head;

    while ((*indirect) != entry)
        indirect = &(*indirect)->next;

    *indirect = entry->next;
    free(entry);
}

void swap_pair(node_t **head)
{
    for (node_t **node = head; *node && (*node)->next;
         node = &((*node)->next->next)) {
        node_t *tmp = *node;
        *node = (*node)->next;
        tmp->next = (*node)->next;
        (*node)->next = tmp;
    }
}

node_t *rev_recursive(node_t *cur)
{
    if (!cur || !(cur->next)) {
        return cur;
    }
    node_t *head = rev_recursive(cur->next);
    cur->next->next = cur;
    cur->next = NULL;
    return head;
}

void reverse(node_t **head)
{
    *head = rev_recursive(*head);
}

void fisher_yates_shuffle(node_t **head)
{
    int len = 0;
    node_t **cur = head;
    node_t **target = NULL;
    node_t *tmp = NULL;
    for (node_t *cur = *head; cur; cur = cur->next) {
        ++len;
    }
    srand(time(NULL));
    for (int i = len - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        target = cur;
        for (int k = 0; k < j; ++k) {
            target = &((*target)->next);
        }
        tmp = (*target);
        *target = *cur;
        *cur = tmp;
        tmp = (*target)->next;
        (*target)->next = (*cur)->next;
        (*cur)->next = tmp;
        cur = &((*cur)->next);
    }
}

void print_list(node_t *head)
{
    for (node_t *current = head; current; current = current->next)
        printf("%d ", current->value);
    printf("\n");
}

int main(void)
{
    node_t *head = NULL;

    print_list(head);

    add_entry(&head, 72);
    add_entry(&head, 101);
    add_entry(&head, 108);
    add_entry(&head, 109);
    add_entry(&head, 110);
    add_entry(&head, 111);

    print_list(head);

    node_t *entry = find_entry(head, 101);
    remove_entry(&head, entry);

    entry = find_entry(head, 111);
    remove_entry(&head, entry);

    print_list(head);

    /* swap pair.
     * See https://leetcode.com/problems/swap-nodes-in-pairs/
     */
    swap_pair(&head);
    print_list(head);

    reverse(&head);
    print_list(head);

    add_entry(&head, 1);
    add_entry(&head, 2);
    add_entry(&head, 3);
    add_entry(&head, 4);
    add_entry(&head, 5);
    add_entry(&head, 6);
    print_list(head);
    fisher_yates_shuffle(&head);
    print_list(head);

    return 0;
}
