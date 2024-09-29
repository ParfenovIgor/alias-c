#pragma once

#include <ordered_array.h>

typedef struct {
    int magic;
    int is_hole;
    int size;
} header_t;

typedef struct {
    int magic;
    header_t *header;
} footer_t;

typedef struct {
    ordered_array_t index;
    void *start_address;
    void *end_address;
} heap_t;

heap_t *create_heap(int heap_size, int heap_index_size);
void *malloc_heap(heap_t *heap, int size);
void free_heap(heap_t *heap, void *p);
