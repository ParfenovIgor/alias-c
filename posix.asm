global posix_open

posix_open:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov rax, 0x2
    mov rdi, [rbp + 16]
    mov rsi, [rbp + 24]
    mov rdx, 0x0
    syscall

    end:

    leave
    ret