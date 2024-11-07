func .test_equal(a #I, b #I) -> #I {
    if (a = b) {
        return 0
    }
    else {
        return 1
    }
}

//* Structs

//* Definition of structs.
//* This way we can declare structs.

typedef Pt := #S{ x: #I, y: #I }

test demo_local_struct {
    def pt := .{ x := 32, y := 76 }
    def a := pt as #I
    return test_equal(a, 76)
}
