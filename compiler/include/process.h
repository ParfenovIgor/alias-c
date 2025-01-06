#pragma once

#include <ast.h>
#include <process.h>
#include <settings.h>

int _execvp(const char *filename, const char *const argv[], const char *const envp[], const char *path);
struct Node *process_parse(const char*, struct Settings*);
struct Node *process_parse_fd(int, struct Settings*);
int process(struct Settings*);
