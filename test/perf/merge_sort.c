#include <stdio.h>
#include <stdlib.h>

void merge_sort(long *a, long lx, long rx) {
    if (rx - lx > 1) {
        long m = (lx + rx) / 2;
        merge_sort(a, lx, m);
        merge_sort(a, m, rx);

        long *b = _malloc((rx - lx) * sizeof(long));
        long ptr1 = lx, ptr2 = m, ptr3 = 0;
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
    long n = 1000000;
    long *a = _malloc(n * sizeof(long));
    long seed = 123;
    for (long i = 0; i < n; i++) {
        a[i] = _rand(seed);
        seed = a[i];
    }

    merge_sort(a, 0, n);

    for (long i = 0; i + 1 < n; i++) {
        if (a[i] > a[i + 1]) {
            return 1;
        }
    }

    return 0;
}