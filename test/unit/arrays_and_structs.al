func .test_equal(a #I, b #I) -> #I
    if (a = b) 0 else 1

//* Arrays and Structs

//* Definition of array
//* We declare and array with `[ ]` syntax. Array should not be empty, and all its elements should have equal types. The array will not be automatically casted to pointer of its element type.

test demo_definition_of_array {
    def arr #A[3]#I := [5, 3, 8]
    def ptr := arr& as #1I
    ptr[2]& <- 7
    return test_equal(ptr[1] + ptr[2], 10)
}

//* Definition of struct
//* We declare instances of structs with `.{ }` syntax. The type of struct instance is inferred. Two struct types with different type names, but same type contents are equal. We can get the struct field value from a struct pointer with `->` syntax. We can get the pointer to the struct field from a struct pointer with `-><field>&` syntax, which is often used to store value in a struct field with `<-` operator.

typedef Pt := #S{ x: #I, y: #I }

test demo_definition_of_struct {
    def pt1 := .{
        x := 32,
        y := 76
    }
    pt1&->x& <- 21
    def pt2 #Pt := pt1
    return test_equal(pt1&->x, pt2&->x)
}

//* Passing structs to functions
//* We can't pass structs to functions (currently). We can return struct from functions, but the implementation <b>doesn't comply with the System V ABI</b>.

func .store_number(s #1Pt, n #I) -> #V {
    s->x& <- n
}

test demo_passing_structs_to_functions {
    def pt1 #Pt
    eval store_number(pt1&, 4)
    return test_equal(pt1&->x, 4)
}
