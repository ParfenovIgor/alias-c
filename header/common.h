#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#define NULL 0

typedef int bool;
#define true 1
#define false 0

char *to_string(int n);

void print_string(int fd, const char *str);
void print_string2(int fd, const char *str1, const char *str2);
void print_string3(int fd, const char *str1, const char *str2, const char *str3);
void print_stringi(int fd, const char *str1, int x, const char *str2);
void print_int(int fd, int n);

char *_strcpy(char *a, char *b);
char *_strdup(const char *a);
char *_strndup(const char *a, int n);
int _strcmp(const char *a, const char *b);
int _strlen(const char *a);
char *concat(const char *a, const char *b);
const char *substr(const char *a, int n);

void *_malloc(int sz);
void _free(void *ptr);

#endif // COMMON_H_INCLUDED