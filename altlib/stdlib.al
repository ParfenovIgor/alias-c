include altlib."string.al"

//* stdlib

//* todo
proto ._malloc(sz #I) -> #1I
proto ._free(ptr #1I) -> #V

func ^.itoa_(n #I) -> #1C {
    return if (n = 0) {
        def str := _malloc(2 * $#C) as #1C
        str[0]& <- '0'
        str[1]& <- '\0'
        return str
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
            m := m / 10
        }
        eval if (sign = -1) {
            len := len + 1
        }

        def str := _malloc((len + 1) * $#C) as #1C
            eval if (sign = -1) {
            str[0]& <- '-'
        }
        str[len]& <- '\0'
        len := len - 1
        eval while (n) {
            def val := n % 10 as #C
            str[len]& <- val + '0'
            n := n / 10
            len := len - 1
        }
        return str
    }
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
