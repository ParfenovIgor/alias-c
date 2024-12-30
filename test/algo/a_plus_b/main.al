include altlib."posix.al"
include altlib."stdio.al"

func ^._start() -> #V {
    eval puti_(readi_() + readi_())
    eval posix_exit(0)
}
