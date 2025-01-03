//* cassert

include altlib."posix.al"

//* assert_
//* Gets one integer value and stop execution with return code `3`, if the value is `0`.
func ^.assert_(x #I) -> #V {
    eval if (not x) { eval posix_exit(3) }
}
