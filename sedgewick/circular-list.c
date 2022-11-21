#include <stdio.h>
#include <stdlib.h>

#define new(type) ((type*)malloc(sizeof(type)))

typedef struct Node { int val; struct Node* next; } Node;

Node* new_circular_list(int val) {
    Node* node = new(Node);
    *node = (Node){ .val = val, .next = node };
    return node;
}

void circular_list_println(Node* list) {
    Node* first = list;
    Node* cur = first;
    printf("[ ");
    do {
        printf("%d ", cur->val);
        cur = cur->next;
    } while(cur != first);
    printf("]\n");
}

Node* circular_list_prev_node(Node* node) {
    Node* prev = node;
    while(prev->next != node) prev = prev->next;
    return prev;
}

void circular_list_append(Node* list, int val) {
    Node* node = new(Node);
    *node = (Node){ .val = val, .next = list };
    circular_list_prev_node(list)->next = node;
}

Node* circular_list_walk_n(Node* list, int n) {
    for(int i = 0; i < n; i++) list = list->next;
    return list;
}

int main() {
    Node* list = new_circular_list(0);
    circular_list_println(list);
    circular_list_append(list, 1);
    circular_list_append(list, 2);
    circular_list_println(list);

    list = circular_list_walk_n(list, 1);
    circular_list_println(list);
    list = circular_list_walk_n(list, 1);
    circular_list_println(list);
    return 0;
}
