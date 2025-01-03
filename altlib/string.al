//* string

include altlib."stdlib.al"

//* strcpy_
//* Copies the string `b` into buffer `a`. Returns `a`.
func ^.strcpy_(a #1C, b #1C) -> #1C {
    def i := 0
    return while (1) {
        a[i]& <- b[i]
        eval if (b[i] = '\0') { break a }
        i := i + 1
    } else a
}

//* strncpy_
//* Copies at most `n` bytes from string `b` into buffer `a`. Returns `a`.
func ^.strncpy_(a #1C, b #1C, n #I) -> #1C {
    def i := 0
    return while (i < n) {
        a[i]& <- b[i]
        eval if (b[i] = '\0') { break a }
        i := i + 1
    } else a
}

//* strcmp_
//* Compares strings `a` and `b` lexicographically. Returns `-1` if `a < b`. Returns `1` if `a > b`. Returns `0` if `a = b`.
func ^.strcmp_(a #1C, b #1C) -> #I {
    def i := 0
    return while (1) {
        eval if (a[i] < b[i]) { break -1 }
        eval if (b[i] < a[i]) { break 1 }
        eval if (a[i] = '\0' or b[i] = '\0') { break 0 }
        i := i + 1
    } else 0
}

//* strncmp_
//* Compares at most `n` bytes of strings `a` and `b` lexicographically. Returns `-1` if `a < b`. Returns `1` if `a > b`. Returns `0` if `a = b`. Here at most `n` bytes of strings `a` and `b` are meant.
func ^.strncmp_(a #1C, b #1C, n #I) -> #I {
    def i := 0
    return while (i < n) {
        eval if (a[i] < b[i]) { break -1 }
        eval if (b[i] < a[i]) { break 1 }
        eval if (a[i] = '\0' or b[i] = '\0') { break 0 }
        i := i + 1
    } else 0
}

//* strlen_
//* Calculates the length of string `a` and returns it.
func ^.strlen_(a #1C) -> #I {
    def i := 0
    return while (1) {
        eval if (a[i] = '\0') { break i }
        i := i + 1
    } else 0
}

//* strnlen_
//* Calculates the length of string `a`, but checks at most `n` bytes. Returns the length of `a` or `n` if it is smaller than length of `a`.
func ^.strnlen_(a #1C, n #I) -> #I {
    def i := 0
    return while (i < n) {
        eval if (a[i] = '\0') { break i }
        i := i + 1
    } else n
}
