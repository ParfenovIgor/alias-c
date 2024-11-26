func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* While

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

//* While else block
//* The `else` block will be executed, if we exit the loop when checking a condition, which is false.

test demo_while_else_block {
    def i := 0
    def sum := 0
    def res := while (i = 0) {
        eval if (i = 10) { break sum }
        sum := sum + i
        i := i + 1
    }
    else 0
    return test_equal(res, 0)
}

//* While continue statement
//* We can jump to the start of the loop using a `continue` statement.

test demo_while_continue_statement {
    def i := 0
    def sum := 0
    def res := while (1) {
        i := i + 1
        eval if (i < 5) { continue }
        eval if (i = 10) { break sum }
        sum := sum + i
    }
    else 0
    return test_equal(res, 35)
}

//* While control with labels
//* We can use `break` and `continue` not only to nearest loop, but also to given labeled loop.

test demo_while_control_with_labels {
    def a := while .foo (1) {
        eval while (1) {
            break .foo 32
        }
    }
    else 0
    
    def i := 0
    def b := while .foo (1) {
        eval if (i = 3) { break 43 }
        eval while (1) {
            i := i + 1
            continue .foo
        }
    }
    else 0

    return test_equal(a - b, -11)
}
