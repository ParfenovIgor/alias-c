func .test_equal(a <int, 0>, b <int, 0>) -> <int, 0> {
    if (a = b) {
        return 0
    }
    else {
        return 1
    }
}

test demo_if_statement {
    def a <int, 0>
    a := 3
    if (a = 2) {
        a := 1
    }
    else if (a = 3) {
        a := 2
    }
    else {
        a := 3
    }
    return .test_equal(a, 2)
}

test demo_while_statement {
    def i <int, 0>; i := 0
    def sum <int, 0>; sum := 0
    while (i < 10) {
        sum := sum + i
        i := i + 1
    }
    return .test_equal(sum, 45)
}
