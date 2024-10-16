include "include/stdio.hal"
include "include/stdlib.hal"
include "../altlib/include/posix.hal"
include "../altlib/include/algorithm.hal"

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

func Point.foo(this <Point, 1>, a <int, 0>) -> <int, 0> {
    this->x& <- a * a
}

struct Pair {
    p1 <Point, 1>
    p2 <Point, 1>
}

func Pair.init(
    this <Pair, 1>,
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

func Pair.apply_foo(this <Pair, 1>, a <int, 0>, b <int, 0>) -> <int, 0> {
    def _ <int, 0>
    _ := this->p1.foo(a)
    _ := this->p2.foo(b)
}

func .putstrs(size <int, 0>, strs <char, 2>) -> <int, 0> {
    def _ <int, 0>
    def i <int, 0>
    i := 0
    while (i < size) {
        _ := ._puts(strs[i]);
        i := i + 1
    }
}

func ^._start() -> <int, 0> {
    def _ <int, 0>

    def str <char, 1>
    str := "abacaba"
    _ := ._puts(str)
    str[3]& <- 'x'
    _ := ._puts(str)

    def bcd <int, 1>
    bcd := [1, 2, 6, 3]
    _ := ._puti(bcd[0])
    _ := ._puti(bcd[1])
    _ := ._puti(bcd[2])
    _ := ._puti(bcd[3])

    def strs <char, 2>
    strs := ["Hello", " world\n"]
    _ := .putstrs(2, strs);

    if (0) {
        _ := ._puti(1)
    }
    else if (1) {
        _ := ._puti(2)
    }
    else {
        _ := ._puti(3)
    }
    
    if (1) {
        _ := ._puti(1)
    }
    else if (0) {
        _ := ._puti(2)
    }

    def c <char, 0>
    c := 'a' + '\n'
    _ := ._puti('a' as <int, 0>)
    def d <char, 1>
    d := ._malloc(^<char, 0> * 100) as <char, 1>
    d[0]& <- 'H'
    d[1]& <- 'e'
    d[2]& <- 'l'
    d[3]& <- 'l'
    d[4]& <- 'o'
    d[5]& <- '\0'
    _ := ._puts(d)

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

    a := ._malloc(^<Point, 0> * 10) as <Point, 1>
    i := 0
    while (i < 10) {
        _ := a[i]&.foo(i)
        i := i + 1
    }
    i := 0
    while (i < 10) {
        _ := ._puti(a[i]&->x)
        i := i + 1
    }

    def x <int, 0>; x := 32
    def y <int, 0>; y := 48
    _ := ._puti(.al_min(x, y))

    // _ := .posix_fork();
    _ := .foo(1234);
    _ := ._puti(_)

    _ := .posix_exit(0)
}
