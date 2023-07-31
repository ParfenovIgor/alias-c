all:
	gcc main.c process.c lexer.c syntax.c compile.c common.c vector.c settings.c token.c exception.c -o calias

.PHONY: test

test:
	./calias test/main.al -m -a
	gcc test/malloc.c -c -o test/malloc.o
	ld test/main.o test/malloc.o -o test/main

run:
	./test/main