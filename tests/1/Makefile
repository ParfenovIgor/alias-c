all:
	../../build/calias -a main.al
	nasm -f elf64 posix.asm
	nasm -f elf64 foo.asm
	ld main.o posix.o foo.o -o program
	rm main.asm main.o posix.o foo.o