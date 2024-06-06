global posix_read
global posix_write
global posix_open
global posix_close
global posix_fork
global posix_execve
global posix_exit
global posix_wait4

posix_read:
    push rbp
    mov rbp, rsp

    mov rax, 0x0
    syscall

    leave
    ret

posix_write:
    push rbp
    mov rbp, rsp
    
    mov rax, 0x1
    syscall

    leave
    ret

posix_open:
    push rbp
    mov rbp, rsp

    mov rax, 0x2
    syscall

    leave
    ret

posix_close:
    push rbp
    mov rbp, rsp

    mov rax, 0x3
    syscall

    leave
    ret

posix_fork:
    push rbp
    mov rbp, rsp

    mov rax, 0x39
    syscall

    leave
    ret

posix_execve:
    push rbp
    mov rbp, rsp

    mov rax, 0x3b
    syscall

    leave
    ret

posix_exit:
    push rbp
    mov rbp, rsp

    mov rax, 0x3c
    syscall

    leave
    ret

posix_wait4:
    push rbp
    mov rbp, rsp

    mov rax, 0x3d
    syscall

    leave
    ret
