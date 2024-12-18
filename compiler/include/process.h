#pragma once

#include <ast.h>
#include <process.h>
#include <settings.h>

struct Node *process_parse(const char*, struct Settings*);
struct Node *process_parse_fd(int, struct Settings*);
int process(struct Settings*);
