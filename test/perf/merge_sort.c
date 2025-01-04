#include <stdio.h>
#include <stdlib.h>

void merge_sort(int *a, int lx, int rx) {
    if (rx - lx > 1) {
        int m = (lx + rx) / 2;
        merge_sort(a, lx, m);
        merge_sort(a, m, rx);

        int *b = _malloc((rx - lx) * sizeof(int));
        int ptr1 = lx, ptr2 = m, ptr3 = 0;
        while (ptr3 < rx - lx) {
            if (ptr1 < m && (ptr2 == rx || a[ptr1] < a[ptr2])) {
                b[ptr3] = a[ptr1];
                ptr1 += 1;
            }
            else {
                b[ptr3] = a[ptr2];
                ptr2++;
            }
            ptr3++;
        }
        ptr3 = 0;
        while (ptr3 < rx - lx) {
            a[lx + ptr3] = b[ptr3];
            ptr3++;
        }
    }
}

int main() {
    int n = 1000000;
    int *a = _malloc(n * sizeof(int));
    int seed = 123;
    for (int i = 0; i < n; i++) {
        a[i] = _rand(seed);
        seed = a[i];
    }

    merge_sort(a, 0, n);

    for (int i = 0; i + 1 < n; i++) {
        if (a[i] > a[i + 1]) {
            return 1;
        }
    }

    return 0;
}