include altlib."posix.al"
include altlib."stdio.al"
include altlib."memory.al"
include altlib."test_allocator.al"

def root #I := 31
def root_pw #I := 1048576
def mod #I := 998244353

func ^.memset(dest #1I, ch #I, count #I) -> #1I _memset(dest, ch, count)

func .powmod(a #I, n #I) -> #I {
    def res #I := 1
    eval while (n) {
        eval if (n % 2) {
            res := res * a % mod
        }
        a := a * a % mod
        n := n / 2
    }
    return res
}

func .inv(a #I) -> #I powmod(a, mod - 2)

func .fft(n #I, a #1I, invert #I) -> #V {
    def i := 1
    def j := 0
    eval while (i < n) {
        def bit := n >> 1
        eval while (j >= bit) {
            j := j - bit
            bit := bit >> 1
        }
        j := j + bit
        eval if (i < j) {
            def x := a[i]
            a[i]& <- a[j]
            a[j]& <- x
        }
        i := i + 1
    }

    def len := 2
    eval while (len <= n) {
        def wlen := powmod(root, (mod - 1) / len)
        eval if (invert) {
            wlen := inv(wlen)
        }
        def i := 0
        eval while (i < n) {
            def w := 1
            def j := 0;
            eval while (j < len / 2) {
                def u := a[i + j]
                def v := a[i + j + len / 2] * w % mod
                a[i + j]& <- if (u + v < mod) u + v else u + v - mod
                a[i + j + len / 2]& <- if (u - v >= 0) u - v else u - v + mod
                w := w * wlen % mod
                j := j + 1
            }
            i := i + len;
        }
        len := len << 1
    }

    eval if (invert) {
        def ninv := inv(n)
        def i := 0
        eval while (i < n) {
            a[i]& <- a[i] * ninv % mod
            i := i + 1
        }
    }
}

func ^._start() -> #V {
    def allocator #TestAllocator
    eval allocator&.init(1024 * 1024 * 1024)
    defer eval allocator&.deinit()

    def n := 1048576
    def a := allocator&.alloc(n * $#I) as #1I
    def b := allocator&.alloc(n * $#I) as #1I
    def seed := 123
    def i := 0
    eval while (i < n / 2) {
        a[i]& <- rand_(seed)
        seed := a[i]
        b[i]& <- rand_(seed)
        seed := b[i]
        i := i + 1
    }
    eval while (i < n) {
        a[i]& <- 0
        b[i]& <- 0
        i := i + 1
    }

    eval fft(n, a, 0)
    eval fft(n, b, 0)
    i := 0
    eval while (i < n) {
        a[i]& <- a[i] * b[i] % mod
        i := i + 1
    }
    eval fft(n, a, 1)

    eval posix_exit(0)
}
