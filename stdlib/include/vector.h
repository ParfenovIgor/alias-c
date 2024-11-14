#pragma once

struct Vector {
    void **ptr;
    int size;
    int reserved;
};

struct Vector vnew();
void vdrop(struct Vector*);
void vpush(struct Vector *v, void *x);
void vpop(struct Vector *v);
void *vback(struct Vector *v);
int vsize(struct Vector *v);
void vreverse(struct Vector *v);
