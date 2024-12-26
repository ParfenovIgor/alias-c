#pragma once

#define NULL 0

char *_itoa(int n);
void _init_malloc();
void *_malloc(int sz);
void _free(void *ptr);
int _rand(int seed);
