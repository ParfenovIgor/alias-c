#pragma once

#define NULL 0

void _init_malloc();
void *_malloc(int sz);
void _free(void *ptr);
int _rand();
void _srand(unsigned int seed);
