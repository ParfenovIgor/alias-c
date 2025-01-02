func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Address

//* Get address and movement
//* We can take a pointer to local variable or function argument with `&` syntax. We can store a value by a pointer with `->` syntax.

test demo_get_address_and_movement {
    def a #I
    def ptr := a&
    ptr <- 12
    return test_equal(a, 12)
}

//* Dereference
//* We can get a value from a pointer (or a pointer with a lower degree) with a `$` syntax.

test demo_dereference {
    def a := 3
    def ptr := a&
    a := 7
    return test_equal(ptr$, 7)
}
