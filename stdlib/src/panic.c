#include <panic.h>
#include <posix.h>
#include <stdio.h>

void _panic(const char *err) {
    _puts("PANIC: ");
    _puts(err);
    posix_exit(3);
}
