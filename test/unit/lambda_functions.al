func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Lambda Functions

//* Simple lambda functions
//* The lambda functions don't require to have name. But we can assign them to variables.

test demo_lambda_functions {
    def f := \(x #I) -> #I {
        return x + 1
    }
    return test_equal(f(2), 3)
}

//* Higher order functions
//* We can pass lambda functions to function calls.

test demo_higher_order_functions {
    def twice := \(f #F(#I) -> #I, x #I) -> #I {
        return f(f(x))
    }
    return test_equal(twice(\(x #I) -> #I x * x, 2), 16)
}
