#include <vector.h>
#include <stdlib.h>
#include <memory.h>

void **push_back(void **a, void *x) {
    int sz = 0;
    for (void **b = a; *b != NULL; b++) {
        sz++;
    }
    void **a_new = (void**)_malloc((sz + 2) * sizeof(void*));
    for (int i = 0; i < sz; i++) {
        a_new[i] = a[i];
    }
    a_new[sz] = x;
    a_new[sz + 1] = NULL;
    _free(a);
    return a_new;
}

void **pop_back(void **a) {
    void **b = a;
    while (*(b + 1) != NULL) {
        b++;
    }
    *b = NULL;
    return a;
}

void *get_back(void **a) {
    void **b = a;
    while (*(b + 1) != NULL) {
        b++;
    }
    return *b;
}

int get_size(void **a) {
    int sz = 0;
    for (void **b = a; *b != NULL; b++) {
        sz++;
    }
    return sz;
}

struct Vector vnew() {
    struct Vector v;
    v.ptr = _malloc(sizeof(void*));
    v.size = 0;
    v.reserved = 1;
    return v;
}

void vpush_back(struct Vector *v, void *x) {
    if (v->size + 1 > v->reserved) {
        int new_reserved = v->reserved * 2;
        void **new_ptr = _malloc(new_reserved * sizeof(void*));
        _memcpy(new_ptr, v->ptr, v->reserved * sizeof(void*));
        _free(v->ptr);
        v->ptr = new_ptr;
        v->reserved = new_reserved;
    }
    v->ptr[v->size] = x;
    v->size++;
}

void vpop_back(struct Vector *v) {
    v->size--;
}

void *vback(struct Vector *v) {
    return v->ptr[v->size - 1];
}

int vsize(struct Vector *v) {
    return v->size;
}
