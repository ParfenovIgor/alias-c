include altlib."posix.al"
include altlib."stdio.al"
include altlib."test_allocator.al"

func ^._start() -> #V {
    def allocator := .{
        data := 0 as #1I,
        size := 0,
        reserved := 0
    }
    eval allocator&.init(1024)

    def buffer := allocator&.alloc(128) as #1C
    def a := readi_(buffer)
    def b := readi_(buffer)
    def c := a + b
    eval puti_(c)

    eval allocator&.deinit()
    eval posix_exit(0)
}
