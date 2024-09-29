include {include/stdio.hal}
include {include/stdlib.hal}
include {../altlib/include/posix.hal}
include {../altlib/include/algorithm.hal}

struct Astr {
    x <int, 0>
    y <int, 0>
}

func foo(a <int, 0>) -> <int, 0> {
    func boo(b <int, 0>) -> <int, 0> {
        return b + 4
    }
    def _ <int, 0>;
    _ := ._puti(a);
    def b <int, 0>
    b := .boo(a)
    return b
}

func ^_start() -> <int, 0> {
    def _ <int, 0>

    def pt <int, 1>
    pt := ._malloc(^<int, 0> * 20)
    pt <- 4
    _ := ._puti(pt$)

    def i <int, 0>
    i := 0
    while (i < 10) {
        pt[i]& <- i * i
        i := i + 1
    }
    while (i < 20) {
        def j <int, 0>; j := pt as <int, 0>
        j := j + ^<int, 0> * i
        def pt2 <int, 1>; pt2 := j as <int, 1>
        pt2 <- i
        i := i + 1
    }
    i := 0
    while (i < 20) {
        _ := ._puti(pt[i])
        i := i + 1
    }

    def a <Astr, 1>
    a := ._malloc(^<Astr, 0>) as <Astr, 1>
    a->x& <- 5
    a->y& <- a->x * a->x
    _ := ._puti(a->x)
    _ := ._puti(a->y)
    _ := ._free(a as <int, 1>)

    def x <int, 0>; x := 32
    def y <int, 0>; y := 48
    _ := ._puti(.al_min(x, y))

    // _ := .posix_fork();
    _ := .foo(1234);
    _ := ._puti(_)

    _ := .posix_exit(0)
}
