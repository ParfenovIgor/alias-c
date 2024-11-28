#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <posix.h>

int _puts(const char *str) {
    int res = 0;
    res += _fputs(STDOUT, str);
    res += _fputs(STDOUT, "\n");
    return res;
}

int _puti(int n) {
    int res = 0;
    res += _fputi(STDOUT, n);
    res += _fputs(STDOUT, "\n");
    return res;
}

int _fputs(int fd, const char *str) {
    return posix_write(fd, str, _strlen(str));
}

int _fputs2(int fd, const char *str1, const char *str2) {
    int res = 0;
    res += _fputs(fd, str1);
    res += _fputs(fd, str2);
    return res;
}

int _fputs3(int fd, const char *str1, const char *str2, const char *str3) {
    int res = 0;
    res += _fputs(fd, str1);
    res += _fputs(fd, str2);
    res += _fputs(fd, str3);
    return res;
}

int _fputsi(int fd, const char *str1, int x, const char *str2) {
    int res = 0;
    res += _fputs(fd, str1);
    res += _fputi(fd, x);
    res += _fputs(fd, str2);
    return res;
}

int _fputi(int fd, int n) {
    char *str = _itoa(n);
    int res = posix_write(fd, str, _strlen(str));
    _free(str);
    return res;
}

int _sputs(char *dst, const char *src) {
    int i;
    for (i = 0; src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    return i;
}

int _sputi(char *dst, int n) {
    char *str = _itoa(n);
    _sputs(dst, str);
    int res = _strlen(str);
    _free(str);
    return res;
}
