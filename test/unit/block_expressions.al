func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Block Expressions

//* Return with expression
//* We can return value from a block using `return` statement. All return expressions have to have equal types.

test demo_return_with_expression {
    def a := {
        return 4
    }
    return test_equal(a, 4)
}

//* Return to label
//* We can return to specific block using a label.

test demo_return_to_label {
    def a := { .foo
        eval {
            return .foo 5
        }
    }
    return test_equal(a, 5)
}

//* Return void
//* The expression `{}` is a block, which returns type `#V`. We can use it to break from blocks, which are used as statements (don't return anything and have type `#V`).

test demo_return_from_block {
    def a := 3
    eval {
        a := 2
        return {}
        a := 3
    }
    return test_equal(a, 2)
}
