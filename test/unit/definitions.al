func .test_equal(a #I, b #I) -> #I {
    if (a = b) {
        return 0
    }
    else {
        return 1
    }
}

//* Definitions

//* Definition
//* Use `def` to define a local variable. The variable must have an initial value after := operator.

test demo_definition {
    def a := 5 // this is variable `a`, which has type `integer`
    def b := '4' // this is variable `b`, which has type `char`
    return 0 // to succeed, the test has to return zero
}

//* Types
//* We can write types of local variables explicitly. The types begin with symbol `#`.

test demo_types {
    def a #I := 5 // `#I` is an `integer` type
    def b #C := '4' // `#C` is a `char` type
    return 0
}

//* Type size
//* Operator `^` returns size of the packed type in bytes. However, on stack data occupies more space, as it has to be word-aligned.

test demo_type_size {
    if (^#I = 8) {} else { return 1 } // The size of `#I` is 8 bytes
    if (^#C = 1) {} else { return 1 } // The size of `#C` is 1 byte
    return 0
}
