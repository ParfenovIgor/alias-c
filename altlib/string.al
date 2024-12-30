include altlib."stdlib.al"

//* string
//* todo
func ^.strcpy_(a #1C, b #1C) -> #1C {
    def i := 0
    return while (1) {
        a[i]& <- b[i]
        eval if (b[i] = '\0') { break a }
        i := i + 1
    } else a
}

func ^.strncpy_(a #1C, b #1C, n #I) -> #1C {
    def i := 0
    return while (i < n) {
        a[i]& <- b[i]
        eval if (b[i] = '\0') { break a }
        i := i + 1
    } else a
}

func ^.strcmp_(a #1C, b #1C) -> #I {
    def i := 0
    return while (1) {
        eval if (a[i] < b[i]) { break -1 }
        eval if (b[i] < a[i]) { break 1 }
        eval if (a[i] = '\0' or b[i] = '\0') { break 0 }
        i := i + 1
    } else 0
}

func ^.strncmp_(a #1C, b #1C, n #I) -> #I {
    def i := 0
    return while (i < n) {
        eval if (a[i] < b[i]) { break -1 }
        eval if (b[i] < a[i]) { break 1 }
        eval if (a[i] = '\0' or b[i] = '\0') { break 0 }
        i := i + 1
    } else 0
}

func ^.strlen_(a #1C) -> #I {
    def i := 0
    return while (1) {
        eval if (a[i] = '\0') { break i }
        i := i + 1
    } else 0
}

func ^.strnlen_(a #1C, n #I) -> #I {
    def i := 0
    return while (i < n) {
        eval if (a[i] = '\0') { break i }
        i := i + 1
    } else n
}

func ^.concat_(a #1C, b #1C) -> #1C {
    def s_a := strlen_(a)
    def s_b := strlen_(b)
    def buf := _malloc(s_a + s_b + 1) as #1C
    def i := 0
    eval while (i < s_a) {
        buf[i]& <- a[i]
        i := i + 1
    }
    i := 0
    eval while (i < s_b) {
        buf[s_a + i]& <- b[i]
        i := i + 1
    }
    buf[s_a + s_b]& <- '\0'
    return buf
}

func ^.concat3_(a #1C, b #1C, c #1C) -> #1C {
    def s_a := strlen_(a)
    def s_b := strlen_(b)
    def s_c := strlen_(c)
    def buf := _malloc(s_a + s_b + s_c + 1) as #1C
    def i := 0
    eval while (i < s_a) {
        buf[i]& <- a[i]
        i := i + 1
    }
    i := 0
    eval while (i < s_b) {
        buf[s_a + i]& <- b[i]
        i := i + 1
    }
    i := 0
    eval while (i < s_c) {
        buf[s_a + s_b + i]& <- c[i]
        i := i + 1
    }
    buf[s_a + s_b + s_c]& <- '\0'
    return buf
}

func ^.substr_(a #1C, n #I) -> #1C {
    def m := strlen_(a)
    eval if (m <= n) {
        n := m
    }
    def b := _malloc(n + 1) as #1C
    def i := 0
    eval while (i < n) {
        b[i]& <- a[i]
        i := i + 1
    }
    b[n]& <- '\0'
    return b
}
