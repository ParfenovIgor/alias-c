include altlib."posix.al"
include altlib."stdio.al"
include altlib."vector.al"
include altlib."test_allocator.al"

def n #I
def g #1Vector
def used #1I

func .dfs(v #I) -> #V {
    used[v]& <- 1
    def sz := g[v]&.size()
    def i := 0
    eval while (i < sz) {
        def to := g[v]&.get(i)
        eval if (used[to] = 0) {
            eval dfs(to)
        }
        i := i + 1
    }
};

func ^._start() -> #V {
    def allocator #TestAllocator
    eval allocator&.init(1024 * 1024 * 1024)
    defer eval allocator&.deinit()

    def seed := 123

    n := 1000000
    g := allocator&.alloc(n * $#Vector) as #1Vector
    used := allocator&.alloc(n * $#I) as #1I

    def i := 0
    eval while (i < n) {
        eval g[i]&.init(allocator&)
        used[i]& <- 0
        i := i + 1
    }

    i := 1
    eval while (i < n) {
        def a := i
        seed := rand_(seed)
        def b := seed % i
        eval g[a]&.push(b)
        eval g[b]&.push(a)
        i := i + 1
    }

    eval dfs(0)

    eval posix_exit(0)
}
