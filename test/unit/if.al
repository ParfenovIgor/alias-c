func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* If

//* If statement
//* The `if` statement consists of set of blocks and conditions. The last block may be without a condition.

test demo_if_statement {
    def a := 3
    eval if (a = 2) {
        a := 1
    }
    else if (a = 3) {
        a := 2
    }
    else {
        a := 3
    }
    return test_equal(a, 2)
}

//* If expression
//* The `if` can be used as expression. All branches of `if` have to return equal types, and also `if` has to have an `else` block, if it returns not `#V`.

test demo_if_expression {
    def a := 3
    a := if (a = 2)
        1
    else if (a = 3)
        2
    else
        3
    return test_equal(a, 2)
}
