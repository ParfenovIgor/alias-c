all:
	gcc main.c process.c lexer.c syntax.c compile.c common.c settings.c token.c exception.c -o calias
