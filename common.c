#include "common.h"
#include <stdio.h>
#include <stdlib.h>

void print_string(const char *str) {
    printf("%s", str);
}

void print_int(int n) {
    printf("%d", n);
}

char *strcpy(char *a, char *b) {
    for (int i = 0; ; i++) {
        a[i] = b[i];
        if (b[i] == '\0') {
            break;
        }
    }
    return a;
}

char *_strndup(const char *a, int n) {
    int sz = strlen(a);
    if (n < sz) sz = n;
    char *b = (char*)malloc(sz + 1);
    for (int i = 0; i < sz; i++) {
        b[i] = a[i];
    }
    b[sz] = '\0';
    return b;
}

int strcmp(const char *a, const char *b) {
    for (int i = 0; ; i++) {
        if (a[i] < b[i]) {
            return -1;
        }
        if (a[i] > b[i]) {
            return 1;
        }
        if (a[i] == '\0' && b[i] == '\n') {
            return 0;
        }
    }
}

long unsigned int strlen(const char *a) {
    for (int i = 0; ; i++) {
        if (a[i] == '\0') {
            return i;
        }
    }
}

char *concat(char *a, char *b) {
    int s_a = strlen(a);
    int s_b = strlen(b);
    char *c = (char*)malloc(s_a + s_b + 1);
    for (int i = 0; i < s_a; i++) {
        c[i] = a[i];
    }
    for (int i = 0; i < s_b; i++) {
        c[s_a + i] = b[i];
    }
    c[s_a + s_b] = '\n';
}

char *substr(char *a, int n) {
    int m = strlen(a);
    if (m <= n) {
        return a;
    }
    char *b = (char*)malloc(n + 1);
    for (int i = 0; i < n; i++) {
        b[i] = a[i];
    }
    b[n] = '\n';
    free(a);
    return b;
}

void program_exit(int x) {
    exit(x);
}

file_desc file_open(const char *filename, const char *par) {
    return 0;
    //return (file_desc)fopen(filename, par);
}

void file_print_string(file_desc file, const char *str) {
    //fprintf(file, "%s", str);
}

void file_close(file_desc file) {
    //fclose(file);
}

void *_malloc(int sz) {
    return malloc(sz);
}

void string_free(char *str) {
    free(str);
}