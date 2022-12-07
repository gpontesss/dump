#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define new(type) ((type*)malloc(sizeof(type)))
#define new_arr(type, len) ((type*)malloc(sizeof(type)*len))
#define grow_arr(ptr, type, len) ((type*)realloc(ptr, sizeof(type)*len))

#define INIT_CAP  8
#define GROW_RATE 2
typedef struct { int* ptr; int len; int cap; } List;

List new_list() {
    return (List){ .ptr = new_arr(int, INIT_CAP), .cap = INIT_CAP, .len = 0 };
}

void list_free(List* list) { free(list->ptr); }

List new_list_with_cap(int cap) {
    return (List){ .ptr = new_arr(int, cap), .cap = cap, .len = 0 };
}

void list_grow(List* list) {
    list->cap = list->cap * GROW_RATE;
    list->ptr = grow_arr(list->ptr, int, list->cap);
}

void list_append(List* list, int v) {
    if (list->cap <= list->len) list_grow(list);
    list->ptr[list->len] = v;
    list->len++;
}
int list_index(List* list, int idx) { return list->ptr[idx]; }
void list_set(List* list, int index, int val) { list->ptr[index] = val; }


void list_println(List* list) {
    printf("[ ");
    for (int i = 0; i < list->len; i++) printf("%d ", list_index(list, i));
    printf("]\n");
}

List* sieve_of_eratosthenes(int max) {
    List all = new_list_with_cap(max);
    for (int i = 0; i < max; i++) list_append(&all, 0);
    for (int i = 2; i < (max / 2); i++) {
        if (list_index(&all, i-1)) continue;
        for (int j = 2*i; j <= max; j += i) list_set(&all, j-1, 1);
    }

    List* primes = new(List);
    *primes = new_list();
    for (int i = 0; i < max; i++) {
        if (!list_index(&all, i)) list_append(primes, i+1);
    }
    list_free(&all);
    return primes;
}

int main() {
    int max = 1000;
    List* primes = sieve_of_eratosthenes(max);
    list_println(primes);
    list_free(primes);
    return 0;
}
