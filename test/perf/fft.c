#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

const long root = 31;
const long root_pw = 1048576;
const long mod = 998244353;

long powmod(long a, long n) {
    long res = 1;
    while (n) {
        if (n % 2) {
            res = (res * a) % mod;
        }
        a = (a * a) % mod;
        n /= 2;
    }
    return res;
}

long inv(long a) {
    return powmod(a, mod - 2);
}

void fft(long n, long *a, long invert) {
    for (long i = 1, j = 0; i < n; i++) {
        long bit = n >> 1;
        for (; j >= bit; bit >>= 1) {
            j -= bit;
        }
        j += bit;
        if (i < j) {
            long x = a[i];
            a[i] = a[j];
            a[j] = x;
        }
    }

    for (long len = 2; len <= n; len <<= 1) {
        long wlen = powmod(root, (mod - 1) / len);
        if (invert) wlen = inv(wlen);
        for (long i = 0; i < n; i += len) {
            long w = 1;
            for (long j = 0; j < len / 2; j++) {
                long u = a[i + j];
                long v = (a[i + j + len / 2] * w) % mod;
                a[i + j] = u + v < mod ? u + v : u + v - mod;
                a[i + j + len / 2] = u - v >= 0 ? u - v : u - v + mod;
                w = (w * wlen) % mod;
            }
        }
    }

    if (invert) {
        long ninv = inv(n);
        for (long i = 0; i < n; i++) {
            a[i] = a[i] * ninv % mod;
        }
    }
}

void *memset(void *dest, int ch, int count) {
    return _memset(dest, ch, count);
}

int main() {
    long n = 1048576;
    long *a = _malloc(n * sizeof(long));
    long *b = _malloc(n * sizeof(long));
    long seed = 123;
    for (long i = 0; i < n / 2; i++) {
        a[i] = _rand(seed);
        seed = a[i];
        b[i] = _rand(seed);
        seed = b[i];
    }
    for (long i = n / 2; i < n; i++) {
        a[i] = 0;
        b[i] = 0;
    }

    fft(n, a, 0);
    fft(n, b, 0);
    for (long i = 0; i < n; i++) {
        a[i] = (a[i] * b[i]) % mod;
    }
    fft(n, a, 1);

    return 0;
}