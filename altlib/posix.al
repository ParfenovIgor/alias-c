proto .posix_read(fd #I, buffer #1I, count #I) -> #I
proto .posix_write(fd #I, buffer #1I, count #I) -> #I
proto .posix_open(filename #1C, flags #I, mode #I) -> #I
proto .posix_close(fd #I) -> #I
proto .posix_mmap(start #1I, length #I, prot #I, flags #I, fd #I, offset #I) -> #1I
proto .posix_munmap(start #1I, length #I) -> #I
proto .posix_fork() -> #I
proto .posix_execve(filename #1I, argv #2C, envp #2C) -> #I
proto .posix_exit(error_code #I) -> #I
proto .posix_wait4(pid #I, stat_addr #1I, options #I, rusage #1I) -> #I
proto .posix_unlink(pathname #1I) -> #I
