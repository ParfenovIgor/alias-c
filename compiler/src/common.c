#include <common.h>
#include <posix.h>
#include <stdlib.h>
#include <string.h>

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

int print_string_to_string(char *dest, const char *src) {
    int i;
    for (i = 0; src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    return i;
}

int print_int_to_string(char *dest, int n) {
    char *str = to_string(n);
    print_string_to_string(dest, str);
    int res = _strlen(str);
    _free(str);
    return res;
}
