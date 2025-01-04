global _memcpy
_memcpy:
    call _cpy_dir
    ret

global _memmove
_memmove:
    mov rax, rdi
    sub rax, rsi
    jl _call_dir
    call _cpy_rev
    ret
_call_dir:
    call _cpy_dir
    ret

_cpy_dir:
    mov rcx, rdx
    rep movsb
    mov rax, rdi
    ret

_cpy_rev:
    mov rcx, rdx
    add rdi, rcx
    sub rdi, 1
    add rsi, rcx
    sub rsi, 1
    std
    rep movsb
    mov rax, rdi
    cld
    ret

global _memset
_memset:
    mov rax, rsi
    mov rcx, rdx
    rep stosb
    mov rax, rdi
    ret
