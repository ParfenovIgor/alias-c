#include "../header/common.h"
#include "../header/posix.h"

char *to_string(int n) {
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

void print_string(int fd, const char *str) {
    posix_write(fd, str, _strlen(str));
}

void print_string2(int fd, const char *str1, const char *str2) {
    print_string(fd, str1);
    print_string(fd, str2);
}

void print_string3(int fd, const char *str1, const char *str2, const char *str3) {
    print_string(fd, str1);
    print_string(fd, str2);
    print_string(fd, str3);
}

void print_stringi(int fd, const char *str1, int x, const char *str2) {
    print_string(fd, str1);
    print_int(fd, x);
    print_string(fd, str2);
}

void print_int(int fd, int n) {
    char *str = to_string(n);
    posix_write(fd, str, _strlen(str));
    _free(str);
}

char *_strcpy(char *a, char *b) {
    for (int i = 0; ; i++) {
        a[i] = b[i];
        if (b[i] == '\0') {
            break;
        }
    }
    return a;
}

char *_strdup(const char *a) {
    int sz = _strlen(a);
    char *b = (char*)_malloc(sz + 1);
    for (int i = 0; i < sz; i++) {
        b[i] = a[i];
    }
    b[sz] = '\0';
    return b;
}

char *_strndup(const char *a, int n) {
    int sz = _strlen(a);
    if (n < sz) sz = n;
    char *b = (char*)_malloc(sz + 1);
    for (int i = 0; i < sz; i++) {
        b[i] = a[i];
    }
    b[sz] = '\0';
    return b;
}

int _strcmp(const char *a, const char *b) {
    for (int i = 0; ; i++) {
        if (a[i] < b[i]) {
            return -1;
        }
        if (a[i] > b[i]) {
            return 1;
        }
        if (a[i] == '\0' && b[i] == '\0') {
            return 0;
        }
    }
}

int _strlen(const char *a) {
    for (int i = 0; ; i++) {
        if (a[i] == '\0') {
            return i;
        }
    }
}

char *concat(const char *a, const char *b) {
    int s_a = _strlen(a);
    int s_b = _strlen(b);
    char *c = (char*)_malloc(s_a + s_b + 1);
    for (int i = 0; i < s_a; i++) {
        c[i] = a[i];
    }
    for (int i = 0; i < s_b; i++) {
        c[s_a + i] = b[i];
    }
    c[s_a + s_b] = '\0';
    return c;
}

const char *substr(const char *a, int n) {
    int m = _strlen(a);
    if (m <= n) {
        n = m;
    }
    char *b = (char*)_malloc(n + 1);
    for (int i = 0; i < n; i++) {
        b[i] = a[i];
    }
    b[n] = '\0';
    return b;
}

unsigned long pos = 0;

void *_malloc(int sz) {
    if (pos == 0) {
        asm("mov %%rsp, %0\n"
        : "=r"(pos));
        pos -= 0x100000;
    }
    void *res = (void*)pos;
    pos += sz * sizeof(void*);
    return res;
    //return malloc(sz);
}

void _free(void *ptr) {
    // free(ptr);
}

