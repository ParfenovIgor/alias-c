proto ._malloc(sz <int, 0>) -> <int, 1>
proto ._free(pt <int, 1>) -> <int, 0>

proto ._puts(n <char, 1>) -> <int, 0>
proto ._puti(n <int, 0>) -> <int, 0>

include altlib."posix.al"
include altlib."algorithm.al"
include altlib."string.al"

func ^._start() -> <int, 0> {
    def _ <int, 0>

    def str1 <char, 1>
    str1 := "abacaba"
    _ := ._puts(str1);
    
    def str2 <char, 1>
    str2 := ._malloc(10 * ^<char, 0>) as <char, 1>
    _ := .strcpy_(str2, str1) as <int, 0>
    _ := ._puts(str2);

    def str3 <char, 1>
    str3 := ._malloc(10 * ^<char, 0>) as <char, 1>
    _ := .strncpy_(str3, str1, 3) as <int, 0>
    _ := ._puts(str3);

    def n1 <int, 0>
    n1 := .strcmp_(str1, str2)
    _ := ._puti(n1)

    str2[3]& <- 't'
    def n2 <int, 0>
    n2 := .strcmp_(str1, str2)
    _ := ._puti(n2)

    def n3 <int, 0>
    n3 := .strcmp_(str2, str1)
    _ := ._puti(n3)

    def n4 <int, 0>
    n4 := .strncmp_(str1, str2, 3)
    _ := ._puti(n4)

    def n5 <int, 0>
    n5 := .strncmp_(str1, str2, 4)
    _ := ._puti(n5)

    def n6 <int, 0>
    n6 := .strlen_(str1)
    _ := ._puti(n6)

    def n7 <int, 0>
    n7 := .strlen_(str3)
    _ := ._puti(n7)

    _ := .posix_exit(0)
}
