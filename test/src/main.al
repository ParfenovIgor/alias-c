include {test/include/posix.hal}
include {test/include/stdio.hal}
include {test/include/stdlib.hal}

struct Astr {
    x <int, 0>
    y <int, 0>
}

func foo(a <int, 0>) -> <int, 0> {
    def _ <int, 0>;
    _ := call _puti(a);
    def b <int, 0>
    b := 43
    return b
}

func ^main() -> <int, 0> {
    def _ <int, 0>

    def pt <int, 1>
    pt := call _malloc(^<int, 0> * 10)
    pt <- 4
    _ := call _puti($pt)
    
    def a <Astr, 1>
    a := call _malloc(^<Astr, 0>) as <Astr, 1>
    a->x <- 5
    a->y <- a->x * a->x
    _ := call _free(a as <int, 1>)

    _ := call _puti(a->x)
    _ := call _puti(a->y)

    _ := call posix_fork();
    _ := call foo(1234);
    _ := call _puti(_)
}
