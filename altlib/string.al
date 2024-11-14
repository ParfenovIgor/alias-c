func ^.strcpy_(a #1C, b #1C) -> #1C {
    def i := 0
    while (1) {
        a[i]& <- b[i]
        if (b[i] = '\0') {
            return a
        }
        i := i + 1
    }
}

func ^.strncpy_(a #1C, b #1C, n #I) -> #1C {
    def i := 0
    while (1) {
        a[i]& <- b[i]
        if (b[i] = '\0') {
            return a
        }
        i := i + 1
        if (i = n) {
            return a
        }
    }
}

func ^.strcmp_(a #1C, b #1C) -> #I {
    def i := 0
    while (1) {
        if (a[i] < b[i]) {
            return -1
        }
        if (b[i] < a[i]) {
            return 1
        }
        if (a[i] = '\0') {
            return 0
        }
        if (b[i] = '\0') {
            return 0
        }
        i := i + 1
    }
}

func ^.strncmp_(a #1C, b #1C, n #I) -> #I {
    def i := 0
    while (1) {
        if (a[i] < b[i]) {
            return -1
        }
        if (b[i] < a[i]) {
            return 1
        }
        if (a[i] = '\0') {
            return 0
        }
        if (b[i] = '\0') {
            return 0
        }
        i := i + 1
        if (i = n) {
            return 0
        }
    }
}

func ^.strlen_(a #1C) -> #I {
    def i := 0
    while (1) {
        if (a[i] = '\0') {
            return i
        }
        i := i + 1
    }
}
