global _memcpy
_memcpy:
    mov rcx, rdx
    rep movsb
    mov rax, rdi
    ret

global _memset
_memset:
    mov rsi, rax
    mov rcx, rdx
    rep stosb
    mov rax, rdi
    ret
