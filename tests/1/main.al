include {posix.hal}
include {foo.hal}

func ^main() {
    call posix_fork()
    call foo()
    call posix_fork()
    call foo()
}
