#include <vector.h>
#include <stdlib.h>
#include <memory.h>

struct Vector vnew() {
    struct Vector v;
    v.ptr = _malloc(sizeof(void*));
    v.size = 0;
    v.reserved = 1;
    return v;
}

void vdrop(struct Vector *v) {
    _free(v->ptr);
}

void vpush(struct Vector *v, void *x) {
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

void vpop(struct Vector *v) {
    v->size--;
}

void *vback(struct Vector *v) {
    return v->ptr[v->size - 1];
}

int vsize(struct Vector *v) {
    return v->size;
}

void vreverse(struct Vector *v) {
    for (int i = 0; i < v->size / 2; i++) {
        void *x = v->ptr[i];
        v->ptr[i] = v->ptr[v->size - i - 1];
        v->ptr[v->size - i - 1] = x;
    }
}
