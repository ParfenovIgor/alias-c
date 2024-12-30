include altlib."posix.al"
include altlib."stdlib.al"
include altlib."string.al"

//* stdio

//* fputs_
//* Prints the string `str` to the descriptor `fd`. Returns length of the string printed.
func ^.fputs_(fd #I, str #1C) -> #I
    posix_write(fd, str, strlen_(str))

//* fputs2_
//* Prints the strings `str1` and `str2` to the descriptor `fd`. Returns length of the string printed.
func ^.fputs2_(fd #I, str1 #1C, str2 #1C) -> #I
    fputs_(fd, str1) + fputs_(fd, str2)


//* fputs3_
//* Prints the strings `str1`, `str2` and `str3` to the descriptor `fd`. Returns length of the string printed.
func ^.fputs3_(fd #I, str1 #1C, str2 #1C, str3 #1C) -> #I
    fputs_(fd, str1) + fputs_(fd, str2) + fputs_(fd, str3)

//* fputi_
//* Prints the integer `n` to the descriptor `fd`. Returns length of the string printed.
func ^.fputi_(fd #I, n #I) -> #I {
    def str := itoa_(n)
    def res := posix_write(fd, str, strlen_(str))
    eval _free(str as #1I)
    return res
}

//* fputsi_
//* Prints the string `str1`, integer `x` and string `str2` to the descriptor `fd`. Returns length of the string printed.
func ^.fputsi_(fd #I, str1 #1C, x #I, str2 #1C) -> #I
    fputs_(fd, str1) + fputi_(fd, x) + fputs_(fd, str2)

//* puts_
//* Prints the string `str` to the standard output descriptor. Returns length of the string printed.
func ^.puts_(str #1C) -> #I {
    def STDOUT := 1
    return fputs_(STDOUT, str) + fputs_(STDOUT, "\n")
}

//* puti_
//* Prints the integer `n` to the standard output descriptor. Returns length of the string printed.
func ^.puti_(n #I) -> #I {
    def STDOUT := 1
    return fputi_(STDOUT, n) + fputs_(STDOUT, "\n")
}

//* sputs_
//* Copies the string `src` to string `dst`. Doesn't check for the length. Returns length of the string.
func ^.sputs_(dst #1C, src #1C) -> #I {
    def i := 0
    return while (src[i] <> '\0') {
        dst[i]& <- src[i]
        i := i + 1
    }
    else i
}

//* sputi_
//* Prints the integer `n` to the string `dst`. Doesn't check for the length. Returns length of the string printed.
func ^.sputi_(dst #1C, n #I) -> #I {
    def str := itoa_(n)
    eval sputs_(dst, str)
    def res := strlen_(str)
    eval _free(str as #1I)
    return res
}

//* freadc_
//* Reads and returns one character from the descriptor `fd`.
func ^.freadc_(fd #I) -> #C {
    def c := '\0'
    return if (posix_read(fd, c&, 1) = 0) '\0' else c
}

//* freads_
//* Reads a string from the descriptor `fd` to the string `dst`. Doesn't check for the length. Returns the length of the string. <b>Vulnerable function!</b>
func ^.freads_(fd #I, dst #1C) -> #I {
    def i := 0
    def dsti := dst as #I
    eval while (1) {
        def size := posix_read(fd, (dsti + i) as #1C, 1)
        eval if (size = 0) { break {} }
        def ch := dst[i]
        eval if (ch = ' ' or ch = '\n') {
            eval if (i = 0) {
                i := i - 1
            }
            else {
                break {}
            }
        }
        i := i + 1
    }
    dst[i]& <- '\0'
    return i
}

//* freadi_
//* Reads and returns a non-negative integer from the descriptor `fd`. Doesn't check for overflow.
func ^.freadi_(fd #I) -> #I {
    def x := 0
    def start := 0
    return while (1) {
        def c := freadc_(fd)
        eval if ((c = ' ' or c = '\n') and start = 0) { continue }
        start := 1
        eval if (not (c >= '0' and c <= '9')) { break x }
        def d := (c - '0') as #I
        x := x * 10 + d
    }
    else 0
}

//* readc_
//* Reads and returns a character from the stardard input descriptor.
func ^.readc_() -> #C {
    def STDIN := 0
    return freadc_(STDIN)
}

//* reads_
//* Reads a string character from the stardard input descriptor to `dst`. Returns size of the string. <b>Vulnerable function!</b>
func ^.reads_(dst #1C) -> #I {
    def STDIN := 0
    return freads_(STDIN, dst)
}

//* readi_
//* Reads and returns an integer from the stardard input descriptor.
func ^.readi_() -> #I {
    def STDIN := 0
    return freadi_(STDIN)
}
