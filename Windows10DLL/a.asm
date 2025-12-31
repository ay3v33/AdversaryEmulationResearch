.code

DirectSyscallBridge PROC
    mov r10, rcx         ; 1st argument to R10
    mov eax, edx         ; Syscall ID to EAX
    jmp r8               ; Indirect jump to gadget
DirectSyscallBridge ENDP

END
