func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Arithmetic

//* Addition and subtraction
//* We can add with `+` operator and subtract with `-` operator.

test demo_addition_and_subtraction {
    def a := 4 + 15 // now `a` is equal to 19
    def b := a - 3 // now `b` is equal to 16
    def c := -4 + a - 5 - b + 9 // now `c` is equal to 3
    return test_equal(c, 3)
}

//* Multiplication and division
//* We can multiply with `*` operator, divide with `/` operator and take modulo with `%` operator. These operators have higher precedence, than operators `+` and `-`.

test demo_multiplication_and_division {
    def a := 4 + 3 * 9 + 6 * 2
    return test_equal(a, 43)
}

//* Parenthesis
//* We can set higher precedence to expression by enclosing it into parenthesis.

test demo_parenthesis {
    def a := (4 + 3) * (9 + 6) * 2
    return test_equal(a, 210)
}

//* Unary minus
//* We can use `-` as a unary operator.

test demo_unary_minus {
    def a := 5
    def b := -(4 + -a) * 3
    return test_equal(b, 3)
}
