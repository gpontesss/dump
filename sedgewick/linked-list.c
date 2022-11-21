#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Node { int val; struct Node* next; } Node;
typedef struct { Node* head; } List;
typedef struct { List list; Node* cur; } ListIter;

ListIter list_iter_new(List list) { return (ListIter){ list, list.head }; };
int list_iter_val(ListIter *iter) { return iter->cur->val; } 
int list_iter_next(ListIter *iter) {
    iter->cur = iter->cur->next;
    return (iter->cur != iter->cur->next);
}

Node* node_new() { return (Node*)malloc(sizeof(Node)); }

List list_new() {
    Node* head = node_new();
    Node* tail = node_new();
    head->next = tail; tail->next = tail;
    return (List){ head };
}

int list_is_tail(Node* node) { return (node->next == node); }

Node* list_last_node(List list) {
    Node* last = list.head;
    while(!list_is_tail(last->next)) last = last->next;
    return last;
}

void list_append(List list, int val) {
    Node* last = list_last_node(list);
    Node* new = node_new();
    *new = (Node){ .next = last->next, .val = val };
    last->next = new;
}

int* list_index(List list, unsigned int index) {
    Node* cur = list.head->next;
    for(int i = 0; i < index; i++) {
        cur = cur->next;
        if(list_is_tail(cur)) return NULL;
    }
    return &cur->val;
}

unsigned int list_length(List list) {
    int len;
    Node* cur = list.head->next;
    for(len = 0; !list_is_tail(cur); len++) cur = cur->next;
    return len;
}

void list_println(List list) {
    ListIter iter = list_iter_new(list);
    printf("[ ");
    while(list_iter_next(&iter)) printf("%d ", list_iter_val(&iter));
    printf("]\n");
}


int main() {
    List list = list_new();

    printf("list length: %u\n", list_length(list));
    list_println(list);

    for(int i = 10; i >= 0; i--) list_append(list, i);

    printf("list length: %u\n", list_length(list));
    list_println(list);

    srand(time(NULL));
    unsigned int index = (unsigned int)rand() % list_length(list);
    printf("list[%u] (random index): %d\n", index, *list_index(list, index));
    return 0;
}
