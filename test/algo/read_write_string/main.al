include altlib."posix.al"
include altlib."stdio.al"
include altlib."test_allocator.al"

func ^._start() -> #V {
    def allocator #TestAllocator
    eval allocator&.init(1024)
    defer eval allocator&.deinit()

    def buffer := allocator&.alloc(256) as #1C
    eval reads_(buffer)
    eval puts_(buffer)

    eval posix_exit(0)
}
