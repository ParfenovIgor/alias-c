#include <stdio.h>
#include <stdlib.h>
#include <vector.h>

long n;
struct Vector *g;
long *used;

void dfs(long v) {
    used[v] = 1;
    long sz = vsize(&g[v]);
    for (long i = 0; i < sz; i++) {
        long to = (long)g[v].ptr[i];
        if (used[to] == 0) {
            dfs(to);
        }
    }
}

int main() {
    long seed = 123;

    long n = 1000000;
    g = (struct Vector*)_malloc(n * sizeof(struct Vector));
    used = (long*)_malloc(n * sizeof(long));

    for (long i = 0; i < n; i++) {
        g[i] = vnew();
        used[i] = 0;
    }

    for (long i = 1; i < n; i++) {
        long a = i;
        seed = _rand(seed);
        long b = seed % i;
        vpush(&g[a], (void*)b);
        vpush(&g[b], (void*)a);
    }

    dfs(0);

    return 0;
}
