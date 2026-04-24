include altlib."posix.al"
include altlib."stdio.al"
include altlib."test_allocator.al"

func .floyd(n #I, a #2I) -> #V {
    def k := 0
    eval while (k < n) {
        def i := 0
        eval while (i < n) {
            def j := 0
            eval while (j < n) {
                def x := a[i][k] + a[k][j]
                eval if (a[i][j] > x) {
                    a[i][j]& <- x
                }
                j := j + 1
            }
            i := i + 1
        }
        k := k + 1
    }
}

func ^._start() -> #V {
    def allocator #TestAllocator
    eval allocator&.init(1024 * 1024 * 1024)
    defer eval allocator&.deinit()

    def n := 500
    def a := allocator&.alloc(n * $#1I) as #2I
    def i := 0
    eval while (i < n) {
        a[i]& <- allocator&.alloc(n * $#I) as #1I
        i := i + 1
    }

    def seed := 123
    i := 0
    eval while (i < n) {
        def j := 0
        eval while (j < n) {
            a[i][j]& <- rand_(seed)
            seed := a[i][j]
            j := j + 1
        }
        i := i + 1
    }

    eval floyd(n, a)

    eval posix_exit(0)
}
