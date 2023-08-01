all:
	nasm -f elf64 posix.asm -o posix.o
	gcc main.c process.c lexer.c syntax.c compile.c common.c vector.c settings.c token.c exception.c posix.o -o calias
	rm posix.o

.PHONY: test

test:
	./calias test/main.al -m -a
	gcc test/malloc.c -c -o test/malloc.o
	ld test/main.o test/malloc.o -o test/main

run:
	./test/main