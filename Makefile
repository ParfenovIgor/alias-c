all:
	gcc main.c process.c lexer.c syntax.c compile.c common.c settings.c token.c exception.c -o calias

.PHONY: test

test:
	./calias test/main.al -m -l