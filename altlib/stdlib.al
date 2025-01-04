//* stdlib

include altlib."string.al"

//* itoa_
//* Prints the integer `n` into buffer `dst` in ascii format in radix `radix`. Returns the length of the string. Doesn't check for the length of the buffer.
func ^.itoa_(n #I, dst #1C, radix #I) -> #1C {
    eval if (n = 0) {
        dst[0]& <- '0'
        dst[1]& <- '\0'
    }
    else {
        def sign := 1
        eval if (n < 0) {
            sign := -1
            n := -n
        }

        def len := 0
        def m := n
        eval while (m) {
            len := len + 1
            m := m / radix
        }
        eval if (sign = -1) {
            len := len + 1
        }

        eval if (sign = -1) {
            dst[0]& <- '-'
        }
        dst[len]& <- '\0'
        len := len - 1
        eval while (n) {
            def val := n % radix as #C
            dst[len]& <- val + '0'
            n := n / radix
            len := len - 1
        }
    }
    return dst
}

//* atoi_
//* Converts the integer written in buffer `str` in ascii format into an integer and returns it.
func ^.atoi_(str #1C) -> #I {
    def i := strlen_(str) - 1
    def x := 0
    eval while (i >= 0) {
        def ch := (str[i] - '0') as #I
        x := x * 10 + ch
        i := i - 1
    }
    return x
}

//* rand_
//* Gets a seed `seed` and return the next pseudorandom integer.
func ^.rand_(seed #I) -> #I {
    seed := seed * 1103515245 + 12345
    seed := (seed / 65536) % 32768
    eval if (seed < 0) {
        seed := seed + 32768
    }
    return seed
}
