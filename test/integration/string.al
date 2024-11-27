proto ._malloc(sz #I) -> #1I
proto ._free(pt #1I) -> #I

proto ._puts(n #1C) -> #I
proto ._puti(n #I) -> #I

include altlib."posix.al"
include altlib."algorithm.al"
include altlib."string.al"

func ^._start() -> #V {
    def str1 := "abacaba"
    eval _puts(str1);
    
    def str2 := _malloc(10 * $#C) as #1C
    eval strcpy_(str2, str1) as #I
    eval _puts(str2);

    def str3 := _malloc(10 * $#C) as #1C
    eval strncpy_(str3, str1, 3) as #I
    eval _puts(str3);

    def n1 := strcmp_(str1, str2)
    eval _puti(n1)

    str2[3]& <- 't'
    def n2 := strcmp_(str1, str2)
    eval _puti(n2)

    def n3 := strcmp_(str2, str1)
    eval _puti(n3)

    def n4 := strncmp_(str1, str2, 3)
    eval _puti(n4)

    def n5 := strncmp_(str1, str2, 4)
    eval _puti(n5)

    def n6 := strlen_(str1)
    eval _puti(n6)

    def n7 := strlen_(str3)
    eval _puti(n7)

    eval posix_exit(0)
}
