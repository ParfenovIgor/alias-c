#include <ordered_array.h>
#include <memory.h>
#include <posix.h>

int standard_lessthan_predicate(type_t a, type_t b) {
    return (a < b) ? 1 : 0;
}

ordered_array_t create_ordered_array(int max_size, lessthan_predicate_t less_than) {
    ordered_array_t to_ret;
    to_ret.array = posix_mmap(0, max_size * sizeof(type_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    _memset(to_ret.array, 0, max_size * sizeof(type_t));
    to_ret.size = 0;
    to_ret.max_size = max_size;
    to_ret.less_than = less_than;
    return to_ret;
}

void destroy_ordered_array(ordered_array_t *array) {
    posix_munmap(array->array, array->max_size * sizeof(type_t));
}

bool insert_ordered_array(type_t item, ordered_array_t *array) {
    if (array->size == array->max_size) {
        return false;
    }
    int iterator = 0;
    while (iterator < array->size && array->less_than(array->array[iterator], item))
        iterator++;
    if (iterator == array->size)
        array->array[array->size++] = item;
    else {
        type_t tmp = array->array[iterator];
        array->array[iterator] = item;
        while (iterator < array->size) {
            iterator++;
            type_t tmp2 = array->array[iterator];
            array->array[iterator] = tmp;
            tmp = tmp2;
        }
        array->size++;
    }
    return true;
}

type_t lookup_ordered_array(int i, ordered_array_t *array) {
    return array->array[i];
}

void remove_ordered_array(int i, ordered_array_t *array) {
    while (i < array->size) {
        array->array[i] = array->array[i + 1];
        i++;
    }
    array->size--;
}
