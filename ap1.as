.section .data
msg: .ascii "yolo\n"
msg_len: .quad msg_len - msg


NAME2:

NAME1:
    movq $60, %rax
    movq $99, %rdi
    syscall    

.text
.global _start
_start: 
    movq $1, %rax
    movq $1, %rdi
    movq $msg, %rsi
    movq $-1, %rdx
    syscall
    syscall

    jmp NAME1
