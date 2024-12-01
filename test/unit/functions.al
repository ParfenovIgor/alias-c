func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Functions

//* Functions
//* We can create functions. The function declaration has to be prepended with a `.`. To call a function, use parenthesis `()`. Functions are standard values, and we can store them in any structures. Function have their own types.

func .add_one(x #I) -> #I {
    return x + 1
}

test demo_functions {
    def a := add_one(4)
    def foo := add_one
    def b := foo(2)
    return test_equal(a + b, 8)
}

//* Methods
//* We can create methods for types. The method declaration has to be prepended with a caller type. The first argument of the method has to be the caller type. To call a method, use a `.` syntax. We can not store the method pointer in a variable (for example, this way: `a := b.c()`), since it will cause a capture of the caller value.

func #I.add_one(x #I) -> #I {
    return x + 1
}

func #I.add(x #I, y #I) -> #I {
    return x + y
}

test demo_methods {
    def a := 3
    def b := a.add_one()
    def c := 5.add_one()
    def d := b.add(c)
    return test_equal(d, 10)
}

//* Extern
//* By default all functions are invisible to all source files, which include the file with the function declaration. In this case the function names are also generated, which makes it possible to have multiple functions with the same name in different scopes. We can make a function (or method) external by prepending `^` to its name. In this case, if it is a function, then the symbol will be equal to its name, and if it is a method, then the symbol will be mangled. It is not possible, however, to demonstrate it here.

func ^.foo() -> #I 1

test demo_extern {
    def a := foo()
    def b := 0
    def c := 0
    eval {
        func .foo() -> #I 2 // If you append `^` to this function, the link will fail
        b := foo()
    }
    eval {
        func .foo() -> #I 3
        c := foo()
    }
    return test_equal(a + b + c, 6)
}
