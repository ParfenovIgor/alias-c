#include "../header/vector.h"
#include "../header/common.h"

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

int get_size(void **a) {
    int sz = 0;
    for (void **b = a; *b != NULL; b++) {
        sz++;
    }
    return sz;
}

void *get_back(void **a) {
    void **b = a;
    while (*(b + 1) != NULL) {
        b++;
    }
    return *b;
}
