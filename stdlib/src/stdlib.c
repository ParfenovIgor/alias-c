#include <heap.h>
#include <memory.h>
#include <posix.h>
#include <stdlib.h>
#include <string.h>

char *_itoa(int n) {
    if (n == 0) {
        char *str = (char*)_malloc(sizeof(char) * 2);
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    int sign = 1;
    if (n < 0) {
        sign = -1;
        n = -n;
    }

    int len = 0;
    int m = n;
    while (m) {
        len++;
        m /= 10;
    }
    if (sign == -1) {
        len++;
    }
    char *str = (char*)_malloc(sizeof(char) * (len + 1));
    if (sign == -1) {
        str[0] = '-';
    }
    str[len] = '\0';
    len--;
    while (n) {
        str[len] = (char)(n % 10 + '0');
        n /= 10;
        len--;
    }
    return str;
}

heap_t *heap;

void _init_malloc() {
    int heap_size = 0x10000000;
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

int _rand(int seed) {
    seed = seed * 1103515245 + 12345;
    seed = (seed / 65536) % 32768;
    if (seed < 0) {
        seed += 32768;
    }
    return seed;
}
