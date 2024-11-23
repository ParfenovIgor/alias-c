func .test_equal(a #I, b #I) -> #I {
    if (a = b) {
        return 0
    }
    else {
        return 1
    }
}

//* Block expressions

//* Break with expression
//* We can return value from a block using break statement. All break expressions have to have equal types.

test demo_break_with_expression {
    def a := {
        break 4
    }
    return test_equal(a, 4)
}

//* Break to label
//* We can return to specific block using a label.

test demo_break_to_label {
    def a := { .foo
        {
            break .foo 5
        }
    }
    return test_equal(a, 5)
}

//* Break void
//* The expression `{}` is a block, which returns type `#V`. We can use it to break from blocks, which are used as statements (don't return anything and have type `#V`).

test demo_break_from_block {
    def a := 3
    {
        a := 2
        break {}
        a := 3
    }
    return test_equal(a, 2)
}
