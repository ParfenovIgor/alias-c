//* posix

//* posix_read
//* System call. Reads `count` bytes from descriptor `fd` into `buffer`. Return number of bytes read.
proto .posix_read(fd #I, buffer #1C, count #I) -> #I

//* posix_write
//* System call. Writes `count` bytes from `buffer` into descriptor `fd`. Return number of bytes written.
proto .posix_write(fd #I, buffer #1C, count #I) -> #I

//* posix_open
//* System call. Opens a file and returns the descriptor. Read external documentation.
proto .posix_open(filename #1C, flags #I, mode #I) -> #I

//* posix_close
//* System call. Closes the descriptor `fd`. Returns `0` on success.
proto .posix_close(fd #I) -> #I

//* posix_mmap
//* System call. Perform memory mapping. Read external documentation.
proto .posix_mmap(start #1I, length #I, prot #I, flags #I, fd #I, offset #I) -> #1I

//* posix_munmap
//* System call. Performs memory unmapping from address `start` with size `length`.
proto .posix_munmap(start #1I, length #I) -> #I

//* posix_fork
//* System call. Starts a new process. Returns the `PID` of the child for the parent and `0` for the child.
proto .posix_fork() -> #I

//* posix_execve
//* System call. Starts a program in file `filename` with arguments `args` and environmental variables `envp`. Returns "never type".
proto .posix_execve(filename #1I, argv #2C, envp #2C) -> #I

//* posix_exit
//* System call. Terminates the process with exit code `error_code`. Returns "never type".
proto .posix_exit(error_code #I) -> #I

//* posix_wait4
//* System call. Locks the process until a condition on process `pid` is satisfied. Read external documentation.
proto .posix_wait4(pid #I, stat_addr #1I, options #I, rusage #1I) -> #I

//* posix_getcwd
//* System call. Writes the current working directory to a buffer `buf` with size `size`. Returns `buf`.
proto .posix_getcwd(buf #1C, size #I) -> #1C

//* posix_unlink
//* System call. Unlinks the file `pathname`. Returns `0` on success.
proto .posix_unlink(pathname #1I) -> #I
