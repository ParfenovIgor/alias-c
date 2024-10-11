#pragma once

#include <common.h>

void **push_back(void **a, void *x);
void **pop_back(void **a);
void *get_back(void **a);
int get_size(void **a);

struct Vector {
    void **ptr;
    int size;
    int reserved;
};

struct Vector vnew();
void vdrop(struct Vector*);
void vpush_back(struct Vector *v, void *x);
void vpop_back(struct Vector *v);
void *vback(struct Vector *v);
int vsize(struct Vector *v);