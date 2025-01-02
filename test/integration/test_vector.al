include altlib."algorithm.al"
include altlib."posix.al"
include altlib."stdio.al"
include altlib."stdlib.al"
include altlib."vector.al"

func ^._start() -> #V {
    def vec1 #1Vector := vector_init()
    eval vec1.push(1)
    eval vec1.push(2)
    eval vec1.push(3)
    eval vec1.push(4)

    eval puti_(vec1.get(0))
    eval puti_(vec1.get(1))
    eval puti_(vec1.get(2))
    eval puti_(vec1.get(3))

    eval vec1.pop()
    eval vec1.pop()
    eval vec1.push(5)
    eval vec1.push(6)

    eval puti_(vec1.get(0))
    eval puti_(vec1.get(1))
    eval puti_(vec1.get(2))
    eval puti_(vec1.get(3))

    eval posix_exit(0)
}
