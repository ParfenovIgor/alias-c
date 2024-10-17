#pragma once 

char *to_string(int n);

void print_string(int fd, const char *str);
void print_string2(int fd, const char *str1, const char *str2);
void print_string3(int fd, const char *str1, const char *str2, const char *str3);
void print_stringi(int fd, const char *str1, int x, const char *str2);
void print_int(int fd, int n);
int print_string_to_string(char *dest, const char *src);
int print_int_to_string(char *dest, int n);
