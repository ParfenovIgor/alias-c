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
