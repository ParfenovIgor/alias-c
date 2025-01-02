include altlib."algorithm.al"
include altlib."posix.al"
include altlib."stdio.al"
include altlib."stdlib.al"

typedef Str := #S{ a: #C, b: #C, c: #I, d: #C, e: #I }

func ^._start() -> #V {
    eval puti_($#Str);

    def str1 := .{
        a := 'a',
        b := 'b',
        c := 11,
        d := 'd',
        e := 43
    }

    def str2 #Str := .{
        a := 'A',
        b := 'B',
        c := 111,
        d := 'D',
        e := 143
    }

    str2 := str1
    eval puti_(str1&->c = str2&->c)
    eval puti_(str1&->d = str2&->d)
    
    def t1 := .{
        a := 12,
        b := str1,
        c := 'C'
    }

    def t2 := t1
    eval puti_(str1&->a = str2&->a)
    eval puti_(str1&->c = str2&->c)

    def s1 := t1&->b
    eval puti_(s1&->c)

    def sp := s1&
    def s2 := sp$
    eval puti_(s2&->c)

    s2&->c& <- 100
    s1& <- s2
    eval puti_(s1&->c)

    eval posix_exit(0)
}
