func ^.strcpy_(a <char, 1>, b <char, 1>) -> <char, 1> {
    def i <int, 0>
    i := 0
    while (1) {
        a[i]& <- b[i]
        if (b[i] = '\0') {
            return a
        }
        i := i + 1
    }
}

func ^.strncpy_(a <char, 1>, b <char, 1>, n <int, 0>) -> <char, 1> {
    def i <int, 0>
    i := 0
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

func ^.strcmp_(a <char, 1>, b <char, 1>) -> <int, 0> {
    def i <int, 0>
    i := 0
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

func ^.strncmp_(a <char, 1>, b <char, 1>, n <int, 0>) -> <int, 0> {
    def i <int, 0>
    i := 0
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

func ^.strlen_(a <char, 1>) -> <int, 0> {
    def i <int, 0>
    i := 0
    while (1) {
        if (a[i] = '\0') {
            return i
        }
        i := i + 1
    }
}
