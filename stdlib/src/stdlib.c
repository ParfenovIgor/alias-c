#include "../../stdlib/include/stdlib.h"
#include "../../stdlib/include/heap.h"

heap_t *heap;

void _init_malloc() {
    int heap_size = 0x100000;
    int heap_index_size = 0x1000;
    heap = create_heap(heap_size, heap_index_size);
}

void *_malloc(int sz) {
    if (!heap) {
        _init_malloc();
    }
    return malloc_heap(heap, sz);
}

void _free(void *ptr) {
    free_heap(heap, ptr);
}

unsigned int next = 1;

int _rand() {
    next = next * 1103515245 + 12345;
    return (next / 65536) % 32768;
}

void _srand(unsigned int seed) {
    next = seed;
}
