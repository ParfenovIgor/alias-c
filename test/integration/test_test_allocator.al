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

    def buffer1 := allocator&.alloc(256) as #I
    def buffer2 := allocator&.alloc(256) as #I
    def buffer3 := allocator&.alloc(256) as #I
    def buffer4 := allocator&.alloc(256) as #I
    def buffer5 := allocator&.alloc(256) as #I
    eval puti_(buffer2 - buffer1)
    eval puti_(buffer3 - buffer2)
    eval puti_(buffer4 - buffer3)
    eval puti_(buffer5)

    eval allocator&.deinit()
    eval posix_exit(0)
}
