#include <heap.h>
#include <stdlib.h>

void heap_test() {
    _srand(123);
    const int SZ = 100000;
    void *arr[SZ];
    int sizes[SZ];
    for (int i = 0; i < SZ; i++) arr[i] = 0;

    int heap_size = 0x100000;
    int heap_index_size = 0x1000;

    heap_t *heap = create_heap(heap_size, heap_index_size);
    int memoryUsed = 0;
    int mallocAttempts = 0, mallocSucceeded = 0;
    for (int op = 1; op <= 1000000; op++) {
        if (_rand() % 3) {
            mallocAttempts++;
            int x = (_rand() % 9 + 1) * 4;
            void *ptr = malloc_heap(heap, x);
            if (ptr != 0) {
                mallocSucceeded++;
                memoryUsed += x;
                for (int i = 0; i < SZ; i++) {
                    if (arr[i] == 0) {
                        arr[i] = ptr;
                        sizes[i] = x;
                        break;
                    }
                }
            }
        }
        else {
            for (int i = 0; i < 10; i++) {
                int j = _rand() % SZ;
                if (arr[j] != 0) {
                    free_heap(heap, arr[j]);
                    memoryUsed -= sizes[j];
                    arr[j] = 0;
                    break;
                }
            }
        }
        if (op % 1000 == 0) {
            // printf("Iteration: %d\n", op);
            // printf("Memory used: %f\n", memoryUsed * 1.0 / heap_size);
            // printf("Malloc success rate: %f\n", mallocSucceeded * 1.0 / mallocAttempts);
            // printf("\n");
        }
    }
}
