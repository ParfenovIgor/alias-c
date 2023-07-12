#include "vector.h"

#define NULL 0

struct Node **push_back(struct Node **a, struct Node *node) {
    int sz = 0;
    for (struct Node **b = a; *b != NULL; b++) {
        sz++;
    }
    struct Node **a_new = (struct Node**)_malloc((sz + 2) * sizeof(struct Node*));
    for (int i = 0; i < sz; i++) {
        a_new[i] = a[i];
    }
    a_new[sz] = node;
    a_new[sz + 1] = NULL;
    _free(a);
    return a_new;
}

struct Node **pop_back(struct Node **a) {
    struct Node **b = a;
    while (*(b + 1) != NULL) {
        b++;
    }
    *b = NULL;
    return a;
}

int get_size(struct Node **a) {
    int sz = 0;
    for (struct Node **b = a; *b != NULL; b++) {
        sz++;
    }
    return sz;
}

const char **push_back_string(const char **a, const char *str) {
    int sz = 0;
    for (const char **b = a; *b != NULL; b++) {
        sz++;
    }
    const char **a_new = (const char**)_malloc((sz + 2) * sizeof(const char*));
    for (int i = 0; i < sz; i++) {
        a_new[i] = a[i];
    }
    a_new[sz] = str;
    a_new[sz + 1] = NULL;
    _free(a);
    return a_new;
}

int get_size_string(const char **a) {
    int sz = 0;
    for (const char **b = a; *b != NULL; b++) {
        sz++;
    }
    return sz;
}

const char **pop_back_string(const char **a) {
    const char **b = a;
    while (*(b + 1) != NULL) {
        b++;
    }
    *b = NULL;
    return a;
}

struct Token **push_back_token(struct Token **a, struct Token *token) {
    int sz = 0;
    for (struct Token **b = a; *b != NULL; b++) {
        sz++;
    }
    struct Token **a_new = (struct Token**)_malloc((sz + 2) * sizeof(struct Token*));
    for (int i = 0; i < sz; i++) {
        a_new[i] = a[i];
    }
    a_new[sz] = token;
    a_new[sz + 1] = NULL;
    _free(a);
    return a_new;
}

struct Token *get_back_token(struct Token **a) {
    struct Token **b = a;
    while (*(b + 1) != NULL) {
        b++;
    }
    return *b;
}

struct Token **pop_back_token(struct Token **a) {
    struct Token **b = a;
    while (*(b + 1) != NULL) {
        b++;
    }
    *b = NULL;
    return a;
}

enum Type **push_back_type(enum Type **a, enum Type *type) {
    int sz = 0;
    for (enum Type **b = a; *b != NULL; b++) {
        sz++;
    }
    enum Type **a_new = (enum Type**)_malloc((sz + 2) * sizeof(enum Type*));
    for (int i = 0; i < sz; i++) {
        a_new[i] = a[i];
    }
    a_new[sz] = type;
    a_new[sz + 1] = NULL;
    _free(a);
    return a_new;
}

enum Type **pop_back_type(enum Type **a) {
    enum Type **b = a;
    while (*(b + 1) != NULL) {
        b++;
    }
    *b = NULL;
    return a;
}

bool **push_back_bool(bool **a, bool *x) {
    int sz = 0;
    for (bool **b = a; *b != NULL; b++) {
        sz++;
    }
    bool **a_new = (bool**)_malloc((sz + 2) * sizeof(bool*));
    for (int i = 0; i < sz; i++) {
        a_new[i] = a[i];
    }
    a_new[sz] = x;
    a_new[sz + 1] = NULL;
    _free(a);
    return a_new;
}