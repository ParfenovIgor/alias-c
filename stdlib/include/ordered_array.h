#pragma once

#include <stdbool.h>

typedef void* type_t;
typedef int (*lessthan_predicate_t)(type_t, type_t);

typedef struct {
    type_t *array;
    int size;
    int max_size;
    lessthan_predicate_t less_than;
} ordered_array_t;

int standard_lessthan_predicate(type_t a, type_t b);

ordered_array_t create_ordered_array(int max_size, lessthan_predicate_t less_than);

void destroy_ordered_array(ordered_array_t *array);
bool insert_ordered_array(type_t item, ordered_array_t *array);
type_t lookup_ordered_array(int i, ordered_array_t *array);
void remove_ordered_array(int i, ordered_array_t *array);
