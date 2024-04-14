#include "../../stdlib/include/stdlib.h"

#include <stdlib.h>

void *_malloc(int sz) {
    /* if (pos == 0) {
        asm("mov %%rsp, %0\n"
        : "=r"(pos));
        pos -= 0x800000;
    }
    void *res = (void*)pos;
    pos += sz * sizeof(void*);
    return res; */
    return malloc(sz);
}

void _free(void *ptr) {
    free(ptr);
}
