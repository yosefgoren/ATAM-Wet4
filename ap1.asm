.section .data
.global foo
.global _start
msg: .ascii "yolo\n"
msg_len: .quad msg_len - msg

.text
exit:
    movq $60, %rax
    movq $99, %rdi
    syscall    

foo:
    xor %rax, %rax
    xor %rax, %rax
    xor %rax, %rax
    ret

_start: 
    xor %rax, %rax
    xor %rax, %rax
    xor %rax, %rax

    call foo
    exp_ret_addr:

    jmp exit