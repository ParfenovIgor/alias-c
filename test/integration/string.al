proto ._malloc(sz #I) -> #1I
proto ._free(pt #1I) -> #I

proto ._puts(n #1C) -> #I
proto ._puti(n #I) -> #I

include altlib."posix.al"
include altlib."algorithm.al"
include altlib."string.al"

func ^._start() -> #I {
    def _ := 0

    def str1 := "abacaba"
    _ := _puts(str1);
    
    def str2 := _malloc(10 * ^#C) as #1C
    _ := strcpy_(str2, str1) as #I
    _ := _puts(str2);

    def str3 := _malloc(10 * ^#C) as #1C
    _ := strncpy_(str3, str1, 3) as #I
    _ := _puts(str3);

    def n1 := strcmp_(str1, str2)
    _ := _puti(n1)

    str2[3]& <- 't'
    def n2 := strcmp_(str1, str2)
    _ := _puti(n2)

    def n3 := strcmp_(str2, str1)
    _ := _puti(n3)

    def n4 := strncmp_(str1, str2, 3)
    _ := _puti(n4)

    def n5 := strncmp_(str1, str2, 4)
    _ := _puti(n5)

    def n6 := strlen_(str1)
    _ := _puti(n6)

    def n7 := strlen_(str3)
    _ := _puti(n7)

    _ := posix_exit(0)
}
