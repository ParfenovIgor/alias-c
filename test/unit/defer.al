func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Defer

//* Defer
//* We can execute the statement at the end of the block, if we prepend it with `defer`.

test demo_defer {
    defer return test_equal(a, 2)
    def a := 2
}

//* Multiple defer
//* All deferred statements are executed in reversed order.

test demo_multiple_defer {
    defer return test_equal(a, 2)
    defer eval {
        a := 1
        defer a := 2
        defer a := 3
    }
    def a := 0
}

