func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Definitions

//* Definition
//* Use `def` to define a local variable. The variable must have an initial value after `:=` operator.

test demo_definition {
    def a := 5      // this is variable `a`, which has type `integer`
    def b := '4'    // this is variable `b`, which has type `char`
    return 0        // to succeed, the test has to return zero
}

//* Types
//* We can write types of local variables explicitly. The types begin with symbol `#`.

test demo_types {
    def a #I := 5   // `#I` is an `integer` type
    def b #C := '4' // `#C` is a `char` type
    return 0
}

//* Scopes
//* Local variable only visible in block, where it is defined.

test demo_scopes {
    def a := 2
    eval {
        def a := 3
    }
    return test_equal(a, 2)
}

//* Type size
//* Operator `^` returns size of the packed type in bytes. However, on stack data occupies more space, as it has to be word-aligned.

test demo_type_size { .foo
    // The size of `#I` is 8 bytes
    // The size of `#C` is 1 byte
    return test_equal($#I + $#C, 9)
}

//* Global variables
//* Variable can be created in global scope. If it is not assigned, it will be stored in `.bss` section and assigned to `0`. If it is assigned, it has to be assigned to a simple integer and it will be stored in `.data` section.

def var1 #I
def var2 := 31;

test demo_global_variable {
    var1 := 43
    var1& <- var1 + 6;
    return test_equal(var1 + var2, 80);
}
