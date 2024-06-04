global _memcpy

_memcpy:
	xor rcx, rcx

copy:
	cmp rdx, rcx
	jle decrement_rdi
 	mov r10b, BYTE [rsi + rcx]
 	mov [rdi], r10b
	inc rdi
	inc rcx
	jmp copy

decrement_rdi:
	cmp rcx, 0
	je end
	dec rdi
	dec rcx
	jmp decrement_rdi

end:
	mov rax, rdi
	ret
