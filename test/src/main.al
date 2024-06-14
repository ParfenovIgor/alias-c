include {test/include/posix.hal}
include {test/include/stdio.hal}

struct Astr {
    x <int, 0>
    y <int, 0>
}

func ^main() -> <int, 0> {
    def _ <int, 0>
    _ := call _puti(123)
    _ := call posix_fork();
    _ := call _puti(456)
    // def a <Astr, 1>
    // def b <int, 0>
    // b := a->y
    // a->y <- b
}
