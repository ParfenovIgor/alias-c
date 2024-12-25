include altlib."posix.al"

func ^.assert_(x #I) -> #V {
    eval if (not x) { eval posix_exit(3) }
}
