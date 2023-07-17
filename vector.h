#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include "common.h"

struct Node **push_back(struct Node **a, struct Node *node);
struct Node **pop_back(struct Node **a);

int get_size(struct Node **a);

const char **push_back_string(const char **a, const char *str);
int get_size_string(const char **a);
const char **pop_back_string(const char **a);

struct Token **push_back_token(struct Token **a, struct Token *token);
struct Token *get_back_token(struct Token **a);
struct Token **pop_back_token(struct Token **a);

enum Type **push_back_type(enum Type **a, enum Type *type);
enum Type **pop_back_type(enum Type **a);

int **push_back_int(int **a, int *x);
bool **push_back_bool(bool **a, bool *x);

#endif // VECTOR_H_INCLUDED