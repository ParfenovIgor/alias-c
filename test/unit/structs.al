func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Structs

//* Definition of structs
//* This way we can declare structs.

typedef Pt := #S{ x: #I, y: #I }

test demo_local_struct { .foo
    def pt1 := .{
        x := 32,
        y := 76
    }
    def pt2 := pt1
    eval if (pt1&->x = pt2&->x) {} else { return .foo 1 }
    eval if (pt1&->y = pt2&->y) {} else { return .foo 1 }
    return 0
}

//* Structs layout
//* Structs are packed.

typedef Str := #S{ a: #C, b: #C, c: #I, d: #C, e: #I }

test demo_struct_layout { .foo
    eval if (^#Str = 19) {} else { return .foo 1 }

    def str1 := .{
        a := 'a',
        b := 'b',
        c := 11,
        d := 'd',
        e := 43
    }

    def str2 := .{
        a := 'A',
        b := 'B',
        c := 111,
        d := 'D',
        e := 143
    }

    str2 := str1
    eval if (str1&->c = str2&->c) {} else { return .foo 1 }
    eval if (str1&->d = str2&->d) {} else { return .foo 1 }

    def t1 := .{
        a := 12,
        b := str1,
        c := 'C'
    }

    def t2 := t1
    eval if (str1&->a = str2&->a) {} else { return .foo 1 }
    eval if (str1&->c = str2&->c) {} else { return .foo 1 }

    def s1 := t1&->b
    eval if (s1&->c = 11) {} else { return .foo 1 }

    def sp := s1&
    def s2 := sp$
    eval if (s2&->c = 11) {} else { return .foo 1 }

    s2&->c& <- 100
    s1& <- s2
    eval if (s1&->c = 100) {} else { return .foo 1 }

    return 0
}
