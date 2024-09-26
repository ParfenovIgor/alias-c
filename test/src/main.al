include {test/include/stdio.hal}
include {test/include/stdlib.hal}
include {altlib/include/posix.hal}
include {altlib/include/algorithm.hal}

struct Astr {
    x <int, 0>
    y <int, 0>
}

func foo(a <int, 0>) -> <int, 0> {
    func boo(b <int, 0>) -> <int, 0> {
        return b + 4
    }
    def _ <int, 0>;
    _ := call _puti(a);
    def b <int, 0>
    b := call boo(a)
    return b
}

func ^_start() -> <int, 0> {
    def _ <int, 0>

    def pt <int, 1>
    pt := call _malloc(^<int, 0> * 20)
    pt <- 4
    _ := call _puti($pt)

    def i <int, 0>
    i := 10
    while (i < 20) {
        def j <int, 0>; j := pt as <int, 0>
        j := j + ^<int, 0> * i
        def pt2 <int, 1>; pt2 := j as <int, 1>
        pt2 <- i
        i := i + 1
    }
    i := 10
    while (i < 20) {
        _ := call _puti(pt !! i)
        i := i + 1
    }

    def a <Astr, 1>
    a := call _malloc(^<Astr, 0>) as <Astr, 1>
    a->x <- 5
    a->y <- a->x * a->x
    _ := call _puti(a->x)
    _ := call _puti(a->y)
    _ := call _free(a as <int, 1>)

    def x <int, 0>; x := 32
    def y <int, 0>; y := 48
    _ := call _puti(call al_min(x, y))

    // _ := call posix_fork();
    _ := call foo(1234);
    _ := call _puti(_)

    _ := call posix_exit(0)
}
