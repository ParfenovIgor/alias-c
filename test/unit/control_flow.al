func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Control flow

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

//* While statement
//* The while statement consists of block an condition.

test demo_while_statement {
    def i := 0
    def sum := 0
    eval while (i < 10) {
        sum := sum + i
        i := i + 1
    }
    return test_equal(sum, 45)
}

//* While expression
//* The `while` can be used as expression. All `break` in `while` and an `else` block have to return equal types, and also `while` has to have an `else` block, if it returns not `#V`.

test demo_while_expression {
    def i := 0
    def sum := 0
    def res := while (1) {
        eval if (i = 10) { break sum }
        sum := sum + i
        i := i + 1
    }
    else 0
    return test_equal(res, 45)
}
