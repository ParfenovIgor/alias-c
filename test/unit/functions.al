func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Functions

//* Functions introduction
//* We can create and call functions.

func .add_one(x #I) -> #I {
    return x + 1
}

test demo_functions {
    return test_equal(add_one(1), 2)
}
