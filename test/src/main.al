include {include/stdio.hal}
include {include/stdlib.hal}
include {../altlib/include/posix.hal}
include {../altlib/include/algorithm.hal}

struct Point {
    x <int, 0>
    y <int, 0>
}

func .foo(a <int, 0>) -> <int, 0> {
    func .boo(b <int, 0>) -> <int, 0> {
        return b + 4
    }
    def _ <int, 0>;
    _ := ._puti(a);
    def b <int, 0>
    b := .boo(a)
    return b
}

func Point.foo(a <int, 0>) -> <int, 0> {
    this->x& <- a * a
}

struct Pair {
    p1 <Point, 1>
    p2 <Point, 1>
}

func Pair.init(
    a <int, 0>,
    b <int, 0>,
    c <int, 0>,
    d <int, 0>) -> <int, 0> {
    this->p1& <- ._malloc(^<Point, 0>) as <Point, 1>
    this->p2& <- ._malloc(^<Point, 0>) as <Point, 1>
    this->p1->x& <- a
    this->p1->y& <- b
    this->p2->x& <- c
    this->p2->y& <- d
}

func Pair.apply_foo(a <int, 0>, b <int, 0>) -> <int, 0> {
    def _ <int, 0>
    _ := this->p1.foo(a)
    _ := this->p2.foo(b)
}

func ^._start() -> <int, 0> {
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

    def a <Point, 1>
    a := ._malloc(^<Point, 0>) as <Point, 1>
    a->x& <- 5
    a->y& <- a->x * a->x
    _ := ._puti(a->x)
    _ := ._puti(a->y)
    _ := a.foo(123)
    _ := ._puti(a->x)
    _ := ._free(a as <int, 1>)

    def p <Pair, 1>
    p := ._malloc(^<Pair, 0>) as <Pair, 1>
    _ := p.init(1, 2, 3, 4)
    _ := p.apply_foo(11, 12)
    _ := ._puti(p->p1->x)
    _ := ._puti(p->p2->x)

    def x <int, 0>; x := 32
    def y <int, 0>; y := 48
    _ := ._puti(.al_min(x, y))

    // _ := .posix_fork();
    _ := .foo(1234);
    _ := ._puti(_)

    _ := .posix_exit(0)
}
