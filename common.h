#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

typedef int bool;
#define true 1
#define false 0

void print_string(const char *str);
void print_int(int n);

char *strcpy(char *a, char *b);
char *_strndup(const char *a, int n);
int strcmp(const char *a, const char *b);
long unsigned int strlen(const char *a);
char *concat(char *a, char *b);
char *substr(char *a, int n);

void program_exit(int x);

typedef int file_desc;

file_desc file_open(const char *filename, const char *par);
void file_print_string(file_desc file, const char *str);
void file_close(file_desc file);

void *_malloc(int sz);
void string_free(char *str);

#endif // COMMON_H_INCLUDED