#include "../../stdlib/include/heap.h"
#include "../../stdlib/include/posix.h"
#include <sys/mman.h>

#define HEAP_MAGIC 0x123890AB

void assert(int x) {
    if (!x) {
        posix_exit(3);
    }
}

int find_smallest_hole(int size, heap_t *heap) {
    int iterator = 0;
    while (iterator < heap->index.size) {
        header_t *header = (header_t*)lookup_ordered_array(iterator, &heap->index);
        if (header->size >= size) break;
        iterator++;
    }
    if (iterator == heap->index.size) return -1;
    else return iterator;
}

static int header_t_less_than(void *a, void *b) {
    return (((header_t*)a)->size < ((header_t*)b)->size) ? 1 : 0;
}

heap_t *create_heap(int heap_size, int heap_index_size) {
    heap_t *heap = (heap_t*)posix_mmap(0, sizeof(heap_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    heap->start_address = 0;
    heap->index = create_ordered_array(heap_index_size, &header_t_less_than);
    heap->start_address = posix_mmap(0, heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    heap->end_address = heap->start_address + heap_size;

    header_t *hole = (header_t*)heap->start_address;
    hole->size = heap_size;
    hole->magic = HEAP_MAGIC;
    hole->is_hole = 1;
    insert_ordered_array((void*)hole, &heap->index);

    return heap;
}

void *malloc_heap(heap_t *heap, int size) {
    int new_size = size + sizeof(header_t) + sizeof(footer_t);
    int iterator = find_smallest_hole(new_size, heap);
    if (iterator == -1) {
        return 0;
    }
    
    header_t *orig_hole_header = (header_t*)lookup_ordered_array(iterator, &heap->index);
    void *orig_hole_pos = (void*)orig_hole_header;
    int orig_hole_size = orig_hole_header->size;
    if (orig_hole_size - new_size < sizeof(header_t) + sizeof(footer_t)) {
        size += orig_hole_size - new_size;
        new_size = orig_hole_size;
    }
    remove_ordered_array(iterator, &heap->index);
    header_t *block_header = (header_t*)orig_hole_pos;
    block_header->magic = HEAP_MAGIC;
    block_header->is_hole = 0;
    block_header->size = new_size;
    footer_t *block_footer = (footer_t*)(orig_hole_pos + sizeof(header_t) + size);
    block_footer->magic = HEAP_MAGIC;
    block_footer->header = block_header;
    
    if (orig_hole_size - new_size > 0) {
        header_t *hole_header = (header_t*)(orig_hole_pos + sizeof(header_t) + size + sizeof(footer_t));
        hole_header->magic = HEAP_MAGIC;
        hole_header->is_hole = 1;
        hole_header->size = orig_hole_size - new_size;
        footer_t *hole_footer = (footer_t*)((void*)hole_header + orig_hole_size - new_size - sizeof(footer_t));
        assert((void*)hole_footer < heap->end_address);
        hole_footer->magic = HEAP_MAGIC;
        hole_footer->header = hole_header;
        assert(insert_ordered_array((void*)hole_header, &heap->index));
    }
    return (void*)((void*)block_header + sizeof(header_t));
}

void free_heap(heap_t *heap, void *p) {
    if (p == 0) return;

    header_t *header = (header_t*)((void*)p - sizeof(header_t));
    footer_t *footer = (footer_t*)((void*)header + header->size - sizeof(footer_t));

    assert(header->magic == HEAP_MAGIC);
    assert(footer->magic == HEAP_MAGIC);

    header->is_hole = 1;
    int do_add = 1;

    footer_t *test_footer = (footer_t*)((void*)header - sizeof(footer_t));
    if ((void*)test_footer >= heap->start_address && test_footer->magic == HEAP_MAGIC && test_footer->header->is_hole == 1) {
        int cache_size = header->size;
        header = test_footer->header;
        footer->header = header;
        header->size += cache_size;
        do_add = 0;
    }

    header_t *test_header = (header_t*)((void*)footer + sizeof(footer_t));
    if ((void*)test_header < heap->end_address && test_header->magic == HEAP_MAGIC && test_header->is_hole) {
        header->size += test_header->size;
        test_footer = (footer_t*)((void*)test_header + test_header->size - sizeof(footer_t));
        footer = test_footer;
        footer->header = header;
        int iterator = 0;
        while ((iterator < heap->index.size) &&
                (lookup_ordered_array(iterator, &heap->index) != (void*)test_header))
            iterator++;
        
        assert(iterator < heap->index.size);
        remove_ordered_array(iterator, &heap->index);
    }

    if (do_add == 1) {
        assert(insert_ordered_array((void*)header, &heap->index));
    }
}
