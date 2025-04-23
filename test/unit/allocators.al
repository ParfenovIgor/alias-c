func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

include altlib."test_allocator.al"

//* Allocators

//* Allocator passing
//* If the function signature has @, then this function is an @-function, which expects an allocator to be passed.
//* The allocator can be referenced using @.
//* To call an @-function from a non-@-function (test is simply a function with empty set of arguments), the allocator has to be passed explicitly as first argument, prepended with @.

func .foo@() -> #I {
    return @.alloc(32) as #I
}

test demo_allocator_passing {
    def allocator #TestAllocator
    eval allocator&.init(1024)
    defer eval allocator&.deinit()
    return test_equal(foo(@allocator&) = 0, 0)
}

//* Allocator propagation
//* An @-function can call both @-functions and non-@-functions. For an @-callee the allocator will be propagated.

func .inc(x #I) -> #I {
    return x + 1
}

func .boo@() -> #I {
    return foo() + inc(4)
}

test demo_allocator_propagation {
    def allocator #TestAllocator
    eval allocator&.init(1024)
    defer eval allocator&.deinit()
    return test_equal(boo(@allocator&) = 0, 0)
}

//* Allocator overriding
//* An @-function can call @-function with overriding its allocator with a new one.

func .doo@() -> #I {
    def allocator #TestAllocator
    eval allocator&.init(1024)
    defer eval allocator&.deinit()
    return foo(@allocator&)
}

test demo_allocator_overriding {
    def allocator #TestAllocator
    eval allocator&.init(1024)
    defer eval allocator&.deinit()
    def x := boo(@allocator&)
    def y := doo(@allocator&)
    return test_equal((y - x) = 27, 0)
}
