include altlib."posix.al"
include altlib."stdlib.al"
include altlib."string.al"

func ^.fputs_(fd #I, str #1C) -> #I
    posix_write(fd, str, strlen_(str))

func ^.fputs2_(fd #I, str1 #1C, str2 #1C) -> #I
    fputs_(fd, str1) + fputs_(fd, str2)

func ^.fputs3_(fd #I, str1 #1C, str2 #1C, str3 #1C) -> #I
    fputs_(fd, str1) + fputs_(fd, str2) + fputs_(fd, str3)

func ^.fputi_(fd #I, n #I) -> #I {
    def str := itoa_(n)
    def res := posix_write(fd, str, strlen_(str))
    eval _free(str as #1I)
    return res
}

func ^.fputsi_(fd #I, str1 #1C, x #I, str2 #1C) -> #I
    fputs_(fd, str1) + fputi_(fd, x) + fputs_(fd, str2)

func ^.puts_(str #1C) -> #I {
    def STDOUT := 1
    return fputs_(STDOUT, str) + fputs_(STDOUT, "\n")
}

func ^.puti_(n #I) -> #I {
    def STDOUT := 1
    return fputi_(STDOUT, n) + fputs_(STDOUT, "\n")
}

func ^.sputs_(dst #1C, src #1C) -> #I {
    def i := 0
    return while (src[i] <> '\0') {
        dst[i]& <- src[i]
        i := i + 1
    }
    else i
}

func ^.sputi_(dst #1C, n #I) -> #I {
    def str := itoa_(n)
    eval sputs_(dst, str)
    def res := strlen_(str)
    eval _free(str as #1I)
    return res
}

func ^.freadc_(fd #I) -> #C {
    def c := '\0'
    return if (posix_read(fd, c&, 1) = 0) '\0' else c
}

func ^.freads_(fd #I, dst #1C) -> #I {
    def i := 0
    def dsti := dst as #I
    eval while (1) {
        def size := posix_read(fd, (dsti + i) as #1C, 1)
        eval if (size = 0) { break {} }
        def ch := dst[i]
        eval if (ch = ' ' or ch = '\n') {
            eval if (i = 0) {
                i := i - 1
            }
            else {
                break {}
            }
        }
        i := i + 1
    }
    dst[i]& <- '\0'
    return i
}

func ^.freadi_(fd #I) -> #I {
    def x := 0
    def start := 0
    return while (1) {
        def c := freadc_(fd)
        eval if ((c = ' ' or c = '\n') and start = 0) { continue }
        start := 1
        eval if (not (c >= '0' and c <= '9')) { break x }
        def d := (c - '0') as #I
        x := x * 10 + d
    }
    else 0
}

func ^.readc_() -> #C {
    def STDIN := 0
    return freadc_(STDIN)
}

func ^.reads_(dst #1C) -> #I {
    def STDIN := 0
    return freads_(STDIN, dst)
}

func ^.readi_() -> #I {
    def STDIN := 0
    return freadi_(STDIN)
}
