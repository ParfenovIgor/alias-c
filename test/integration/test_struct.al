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

    eval puti_(str1&->a = 'a' as #I)
    eval puti_(str1&->b = 'b' as #I)
    eval puti_(str1&->c = 11 as #I)
    eval puti_(str1&->d = 'd' as #I)
    eval puti_(str1&->e = 43 as #I)

    def str2 #Str := .{
        a := 'A',
        b := 'B',
        c := 111,
        d := 'D',
        e := 143
    }

    eval puti_(str2&->a = 'A' as #I)
    eval puti_(str2&->b = 'B' as #I)
    eval puti_(str2&->c = 111 as #I)
    eval puti_(str2&->d = 'D' as #I)
    eval puti_(str2&->e = 143 as #I)

    str2 := str1
    
    eval puti_(str2&->a = 'a' as #I)
    eval puti_(str2&->b = 'b' as #I)
    eval puti_(str2&->c = 11 as #I)
    eval puti_(str2&->d = 'd' as #I)
    eval puti_(str2&->e = 43 as #I)

    def t1 := .{
        a := 12,
        b := str1,
        c := 'C'
    }

    eval puti_(t1&->a = 12 as #I)
    eval puti_(t1&->b&->a = 'a' as #I)
    eval puti_(t1&->b&->b = 'b' as #I)
    eval puti_(t1&->b&->c = 11 as #I)
    eval puti_(t1&->b&->d = 'd' as #I)
    eval puti_(t1&->b&->e = 43 as #I)
    eval puti_(t1&->c = 'C' as #I)

    def str3 := t1&->b
    def str4 := str3&
    str4->c& <- 100
    eval puti_(t1&->b&->c)

    def str5 := t1&->b&
    str5->c& <- 100
    eval puti_(t1&->b&->c)

    eval posix_exit(0)
}
