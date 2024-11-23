func .test_equal(a #I, b #I) -> #I {
    if (a = b) {
        return 0
    }
    else {
        return 1
    }
}

//* Control flow

//* If statement
//* The if statement consists of set of blocks and conditions. The last block may be without a condition.

test demo_if_statement {
    def a := 3
    if (a = 2)
        a := 1
    else if (a = 3)
        a := 2
    else
        a := 3
    return test_equal(a, 2)
}

//* While statement
//* The while statement consists of block an condition.

test demo_while_statement {
    def i := 0
    def sum := 0
    while (i < 10) {
        sum := sum + i
        i := i + 1
    }
    return test_equal(sum, 45)
}
