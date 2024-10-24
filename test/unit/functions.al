func .test_equal(a <int, 0>, b <int, 0>) -> <int, 0> {
    if (a = b) {
        return 0
    }
    else {
        return 1
    }
}

//* Functions

//* Functions introduction
//* We can create and call functions.

func .add_one(x <int, 0>) -> <int, 0> {
    return x + 1
}

test demo_functions {
    return .test_equal(.add_one(1), 2)
}
