#include "../../stdlib/include/stdio.h"
#include "../../stdlib/include/stdlib.h"
#include "../../stdlib/include/string.h"
#include "../../stdlib/include/posix.h"

int _puts(const char *str) {
    posix_write(STDOUT, str, _strlen(str));
    posix_write(STDOUT, "\n", 1);
    return 0;
}

int _puti(int n) {
    const int N = 10;
    char arr[N];
    int ptr = N - 1;
    
    if (n == 0) {
        arr[ptr] = '0';
        ptr--;
    }
    else {
        while (n) {
            arr[ptr] = '0' + n % 10;
            ptr--;
            n /= 10;
        }
    }

    ptr++;
    posix_write(STDOUT, arr + ptr, N - ptr);
    posix_write(STDOUT, "\n", 1);
    return 0;
}
