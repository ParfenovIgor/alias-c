global posix_read
global posix_write
global posix_open
global posix_close
global posix_mmap
global posix_munmap
global posix_pipe
global posix_dup2
global posix_fork
global posix_execve
global posix_exit
global posix_wait4
global posix_getcwd
global posix_unlink

posix_read:
    mov rax, 0x0
    syscall
    ret

posix_write:
    mov rax, 0x1
    syscall
    ret

posix_open:
    mov rax, 0x2
    syscall
    ret

posix_close:
    mov rax, 0x3
    syscall
    ret

posix_mmap:
    mov rax, 0x9
    mov r10, rcx
    syscall
    ret

posix_munmap:
    mov rax, 0x0b
    syscall
    ret

posix_pipe:
    mov rax, 0x16
    syscall
    ret

posix_dup2:
    mov rax, 0x21
    syscall
    ret

posix_fork:
    mov rax, 0x39
    syscall
    ret

posix_execve:
    mov rax, 0x3b
    syscall
    ret

posix_exit:
    mov rax, 0x3c
    syscall
    ret

posix_wait4:
    mov rax, 0x3d
    syscall
    ret

posix_getcwd:
    mov rax, 0x4f
    syscall
    ret

posix_unlink:
    mov rax, 0x57
    syscall
    ret
