#include <string.h>
#include <stdio.h>
#include <posix.h>

bool _isalpha(char c) {
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') || c == '_');
}

bool _isdigit(char c) {
    return (c >= '0' && c <= '9');
}

char *_strcpy(char *a, const char *b) {
    for (int i = 0; ; i++) {
        a[i] = b[i];
        if (b[i] == '\0') {
            break;
        }
    }
    return a;
}

char *_strncpy(char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
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

int _strncmp(const char *a, const char *b, int num) {
    for (int i = 0; i < num; i++) {
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
    return 0;
}

int _strlen(const char *a) {
    for (int i = 0; ; i++) {
        if (a[i] == '\0') {
            return i;
        }
    }
}

char *_concat(const char *a, const char *b) {
    int s_a = _strlen(a);
    int s_b = _strlen(b);
    char *buf = (char*)_malloc(s_a + s_b + 1);
    for (int i = 0; i < s_a; i++) {
        buf[i] = a[i];
    }
    for (int i = 0; i < s_b; i++) {
        buf[s_a + i] = b[i];
    }
    buf[s_a + s_b] = '\0';
    return buf;
}

char *_concat3(const char *a, const char *b, const char *c) {
    int s_a = _strlen(a);
    int s_b = _strlen(b);
    int s_c = _strlen(c);
    char *buf = (char*)_malloc(s_a + s_b + s_c + 1);
    for (int i = 0; i < s_a; i++) {
        buf[i] = a[i];
    }
    for (int i = 0; i < s_b; i++) {
        buf[s_a + i] = b[i];
    }
    for (int i = 0; i < s_c; i++) {
        buf[s_a + s_b + i] = c[i];
    }
    buf[s_a + s_b + s_c] = '\0';
    return buf;
}

const char *_substr(const char *a, int n) {
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

char *read_file_descriptor(int fd) {
    const int block = 1000;
    char *contents = (char*)_malloc(sizeof(char));
    contents[0] = '\0';
    int n_blocks = 0;
    while (true) {
        char *buffer = (char*)_malloc(sizeof(char) * (block + 1));
        int cnt = posix_read(fd, buffer, block);
        if (cnt == 0) break;
        buffer[cnt] = '\0';
        char *new_contents = (char*)_malloc(sizeof(char) * ((n_blocks + 1) * block + 1));
        _strcpy(new_contents, contents);
        _strcpy(new_contents + n_blocks * block, buffer);
        _free(contents);
        _free(buffer);
        contents = new_contents;
        n_blocks++;
    }
    return contents;
}

char *read_file(const char *filename) {
    int fd = posix_open(filename, O_RDONLY, 0);
    if (fd <= 0) {
        return NULL;
    }
    char *contents = read_file_descriptor(fd);
    posix_close(fd);
    return contents;
}

int write_file_descriptor(int fd, const char *data) {
    int len = _strlen(data);
    return posix_write(fd, data, len);
}

int write_file(const char *filename, const char *data) {
    int fd = posix_open(filename, O_CREAT | O_WRONLY, 0644);
    int res = _fputs(fd, data);
    posix_close(fd);
    return res;
}
