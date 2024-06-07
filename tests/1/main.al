include {posix.hal}
include {foo.hal}

struct Astr {
    x <int, 0>
    y <int, 0>
}

func ^main() {
    def a <Astr, 1>

    def b <int, 0>
    b := a->y
    
    // a->x = 5
    // ...
    // mov rax, [rsp - 8]
    // mov rbx, [rbp + phase]
    // mov [rbx + phase_struct], rax
    
    // (...)->y
    // ...
    // mov rax, [rsp - 8]
    // mov rbx, [rax + phase_struct]
    // mov [rsp - 8], rbx

    call posix_fork()
    call foo()
    call posix_fork()
    call foo()
}
