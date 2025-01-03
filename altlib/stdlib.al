include altlib."string.al"

//* stdlib

//* todo
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

func ^.rand_(seed #I) -> #I {
    seed := seed * 1103515245 + 12345
    return (seed / 65536) % 32768
}
