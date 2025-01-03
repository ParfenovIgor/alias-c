//* test_allocator

include altlib."posix.al"

typedef TestAllocator := #S {
    data: #1I,
    size: #I,
    reserved: #I
};

//* #1TestAllocator.init
//* Initializes an allocator with size `size` and allocates a buffer.
func ^#1TestAllocator.init(this #1TestAllocator, size #I) -> #V {
    def PROT_READ := 1
    def PROT_WRITE := 2
    def MAP_PRIVATE := 2
    def MAP_ANONYMOUS := 32
    this->data& <- posix_mmap(0 as #1I, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
    this->size& <- 0
    this->reserved& <- size
}

//* #1TestAllocator.deinit
//* Deallocates the buffer of allocator.
func ^#1TestAllocator.deinit(this #1TestAllocator) -> #V {
    eval posix_munmap(this->data, this->reserved)
}

//* #1TestAllocator.alloc
//* Allocates memory of size `size`. Returns the pointer to the memory, or `0` if the buffer has run out.
func ^#1TestAllocator.alloc(this #1TestAllocator, size #I) -> #1I {
    return if (this->size + size <= this->reserved) {
        def x := this->data as #I
        def ptr := (x + this->size) as #1I
        this->size& <- this->size + size
        return ptr
    }
    else 0 as #1I
}
