#include <stdio.h>
#include <stdlib.h>

void floyd(long n, long **a) {
    for (long k = 0; k < n; k++) {
        for (long i = 0; i < n; i++) {
            for (long j = 0; j < n; j++) {
                long x = a[i][k] + a[k][j];
                if (a[i][j] > x) {
                    a[i][j] = x;
                }
            }
        }
    }
}

int main() {
    long n = 500;
    long **a = _malloc(n * sizeof(long*));
    for (long i = 0; i < n; i++) {
        a[i] = _malloc(n * sizeof(long));
    }
    
    long seed = 123;
    for (long i = 0; i < n; i++) {
        for (long j = 0; j < n; j++) {
            a[i][j] = _rand(seed);
            seed = a[i][j];
        }
    }

    floyd(n, a);

    return 0;
}