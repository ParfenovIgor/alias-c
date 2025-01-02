include altlib."algorithm.al"
include altlib."posix.al"
include altlib."stdio.al"
include altlib."stdlib.al"
include altlib."string.al"

func ^._start() -> #V {
    def str1 := "abacaba"
    eval puts_(str1);
    
    def str2 := _malloc(10 * $#C) as #1C
    eval strcpy_(str2, str1) as #I
    eval puts_(str2);

    def str3 := _malloc(10 * $#C) as #1C
    eval strncpy_(str3, str1, 3) as #I
    eval puts_(str3);

    def n1 := strcmp_(str1, str2)
    eval puti_(n1)

    str2[3]& <- 't'
    def n2 := strcmp_(str1, str2)
    eval puti_(n2)

    def n3 := strcmp_(str2, str1)
    eval puti_(n3)

    def n4 := strncmp_(str1, str2, 3)
    eval puti_(n4)

    def n5 := strncmp_(str1, str2, 4)
    eval puti_(n5)

    def n6 := strlen_(str1)
    eval puti_(n6)

    def n7 := strnlen_(str1, 4)
    eval puti_(n7)

    def n8 := strlen_(str3)
    eval puti_(n8)

    def str4 := concat_(str1, str2)
    def str5 := "abac"
    def n9 := strncmp_(str4, str5, 4)
    eval puti_(n9)

    def str6 := substr_(str1, 3)
    def n10 := strcmp_(str6, str3)
    eval puti_(n10)

    eval posix_exit(0)
}
