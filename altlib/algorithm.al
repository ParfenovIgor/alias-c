func ^.min_(a <int, 0>, b <int, 0>) -> <int, 0> {
    if (a < b) {
        return a
    }
    else {
        return b
    }
}

func ^.max_(a <int, 0>, b <int, 0>) -> <int, 0> {
    if (a < b) {
        return b
    }
    else {
        return a
    }
}
