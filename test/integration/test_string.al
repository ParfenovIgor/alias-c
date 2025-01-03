include altlib."algorithm.al"
include altlib."posix.al"
include altlib."stdio.al"
include altlib."stdlib.al"
include altlib."string.al"
include altlib."test_allocator.al"

func ^._start() -> #V {
    def allocator #TestAllocator
    eval allocator&.init(1024)
    defer eval allocator&.deinit()

    def str1 := "abacaba"
    eval puts_(str1);
    
    def str2 := allocator&.alloc(10 * $#C) as #1C
    eval strcpy_(str2, str1) as #I
    eval puts_(str2);

    def str3 := allocator&.alloc(10 * $#C) as #1C
    eval strncpy_(str3, str1, 3) as #I
    eval puts_(str3);

    eval puti_(strcmp_(str1, str2))
    str2[3]& <- 't'
    eval puti_(strcmp_(str1, str2))
    eval puti_(strcmp_(str2, str1))
    eval puti_(strncmp_(str1, str2, 3))
    eval puti_(strncmp_(str1, str2, 4))
    eval puti_(strlen_(str1))
    eval puti_(strnlen_(str1, 4))
    eval puti_(strlen_(str3))

    eval posix_exit(0)
}
