func ^.min_(a #I, b #I) -> #I {
    if (a < b) {
        return a
    }
    else {
        return b
    }
}

func ^.max_(a #I, b #I) -> #I {
    if (a < b) {
        return b
    }
    else {
        return a
    }
}
