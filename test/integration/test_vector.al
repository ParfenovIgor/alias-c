include altlib."algorithm.al"
include altlib."posix.al"
include altlib."stdio.al"
include altlib."stdlib.al"
include altlib."vector.al"

func ^._start() -> #V {
    def allocator #TestAllocator
    eval allocator&.init(1024)
    defer eval allocator&.deinit()

    def vec #Vector
    eval vec&.init(allocator&)
    eval vec&.push(1)
    eval vec&.push(2)
    eval vec&.push(3)
    eval vec&.push(4)

    eval puti_(vec&.size())
    eval puti_(vec&.get(0))
    eval puti_(vec&.get(1))
    eval puti_(vec&.get(2))
    eval puti_(vec&.get(3))

    eval vec&.pop()
    eval vec&.pop()
    eval vec&.push(5)
    eval vec&.push(6)

    eval puti_(vec&.size())
    eval puti_(vec&.get(0))
    eval puti_(vec&.get(1))
    eval puti_(vec&.get(2))
    eval puti_(vec&.get(3))

    eval posix_exit(0)
}
