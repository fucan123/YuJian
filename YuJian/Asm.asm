.CODE

Asm_Nd PROC
    mov eax,edx ;���ܺ�
    xor r9d,r9d  
    xor r8d,r8d  
    mov edx,11h ;��ֹ������
    mov r10,rcx
	
	syscall

	ret
Asm_Nd ENDP

Asm_Rip PROC
    mov rax,[rsp]
	ret
Asm_Rip ENDP

END