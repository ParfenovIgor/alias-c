#pragma once

struct Type {
    const char *identifier;
    int degree;
};

struct Type *BuildType(const char *id, int deg);
struct Type *CopyType(struct Type *type);
