func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Typedef

//* Int type
//* We can give names to types.

typedef T := #I

func .add_one(x #T) -> #T {
    return x + 1
}

test demo_typedef_int {
    return test_equal(add_one(1), 2)
}

//* Function type
//* The function is also a type.

typedef K := #F(#I) -> #I

func .apply(f #K, a #T) -> #T {
    return f(a)
}

test demo_typedef_function {
    return test_equal(apply(add_one, 1), 2)
}
