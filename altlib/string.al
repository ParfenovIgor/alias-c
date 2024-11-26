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
    return while (1) {
        a[i]& <- b[i]
        eval if (b[i] = '\0') { break a }
        i := i + 1
        eval if (i = n) { break a }
    } else a
}

func ^.strcmp_(a #1C, b #1C) -> #I {
    def i := 0
    return while (1) {
        eval if (a[i] < b[i]) { break -1 }
        eval if (b[i] < a[i]) { break 1 }
        eval if (a[i] = '\0') { break 0 }
        eval if (b[i] = '\0') { break 0 }
        i := i + 1
    } else 0
}

func ^.strncmp_(a #1C, b #1C, n #I) -> #I {
    def i := 0
    return while (1) {
        eval if (a[i] < b[i]) { break -1 }
        eval if (b[i] < a[i]) { break 1 }
        eval if (a[i] = '\0') { break 0 }
        eval if (b[i] = '\0') { break 0 }
        i := i + 1
        eval if (i = n) { break 0 }
    } else 0
}

func ^.strlen_(a #1C) -> #I {
    def i := 0
    return while (1) {
        eval if (a[i] = '\0') { break i }
        i := i + 1
    } else 0
}
