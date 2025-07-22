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

    eval puti_(str1&->a = 'a')
    eval puti_(str1&->b = 'b')
    eval puti_(str1&->c = 11)
    eval puti_(str1&->d = 'd')
    eval puti_(str1&->e = 43)

    def str2 #Str := .{
        a := 'A',
        b := 'B',
        c := 111,
        d := 'D',
        e := 143
    }

    eval puti_(str2&->a = 'A')
    eval puti_(str2&->b = 'B')
    eval puti_(str2&->c = 111)
    eval puti_(str2&->d = 'D')
    eval puti_(str2&->e = 143)

    str2 := str1
    
    eval puti_(str2&->a = 'a')
    eval puti_(str2&->b = 'b')
    eval puti_(str2&->c = 11)
    eval puti_(str2&->d = 'd')
    eval puti_(str2&->e = 43)

    def t1 := .{
        a := 12,
        b := str1,
        c := 'C'
    }

    eval puti_(t1&->a = 12)
    eval puti_(t1&->b&->a = 'a')
    eval puti_(t1&->b&->b = 'b')
    eval puti_(t1&->b&->c = 11)
    eval puti_(t1&->b&->d = 'd')
    eval puti_(t1&->b&->e = 43)
    eval puti_(t1&->c = 'C')

    def str3 := t1&->b
    def str4 := str3&
    str4->c& <- 100
    eval puti_(t1&->b&->c)

    def str5 := t1&->b&
    str5->c& <- 100
    eval puti_(t1&->b&->c)

    eval posix_exit(0)
}
