#include <cassert.h>
#include <posix.h>

void _assert(bool x) {
    if (!x) {
        posix_exit(3);
    }
}
