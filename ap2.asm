.section .data

foo_msg: .ascii "foo\n\0"
finish_msg: .ascii "finishing\n\0"
endl_msg: .ascii "\n\0"

.global foo
.global printstr
.global printint
.global _start
.text

strlen: #rdi <-- str_addr, rax <-- return value
    push %rbx

    xor %rax, %rax #counter <-- 0
    strlen_is_char_term:
    movb (%rdi), %bl #tmp <-- *(addr)
    cmp $0, %bl #tmp =? '\0'
    je strlen_finish #if(*(str+counter) == '\0') return counter
    
    inc %rdi #addr++
    inc %rax #counter++
    jmp strlen_is_char_term

    strlen_finish:

    pop %rbx
    ret

printlen:#rdi <-- addr, rsi <-- len
    push %rax
    push %rdx

    movq %rsi, %rdx
    movq %rdi, %rsi
    movq $1, %rax
    movq $1, %rdi

    push %rcx
    push %r11
    syscall
    pop %r11
    pop %rcx

    pop %rdx
    pop %rax
    ret

printstr:#rdi <-- addr_str
    push %rax

    push %rdi
    call strlen
    movq %rax, %rsi
    pop %rdi
    call printlen
    
    pop %rax
    ret

count_dec_digits:#rdi <-- quad_num, ret_value <-- rax
    
    push %rbx
    xor %rbx, %rbx #counter <-- 0
    push %rcx
    movl $10, %ecx
    push %rdx
    mov %rdi, %rax #cur_num <-- rdi

    cdd_check_digit:

    cmp $0, %rax
    je cdd_finish

    # divide quad_num by 10 and increment counter:
    xor %rdx, %rdx
    div %ecx #cur_num <-- cur_num/10
    inc %rbx #counter++
    jmp cdd_check_digit
    
    cdd_finish:
    mov %rbx, %rax
    pop %rdx
    pop %rcx
    pop %rbx
    ret

iotadec:#rdi <-- num, rsi <-- str_dst 
    # '0' == 48 == 0x30
    push %rcx
    movl $10, %ecx
    push %rdx
    push %rax
    mov %rdi, %rax #cur_num <-- rdi

    dec %rsi #str_dst--
    movb $0, (%rsi) #null terminate the digits string.

    atoi_check_digit:
    cmp $0, %rax
    je atoi_finish

    # divide quad_num by 10, put the remainder char in str_dst and increment the dst:
    xor %rdx, %rdx
    div %ecx #cur_num <-- cur_num/10
    addl $48, %edx #edx <-- remainder in ascii
    dec %rsi #str_dst--
    movb %dl, (%rsi)
    jmp atoi_check_digit
    
    atoi_finish:
    pop %rax
    pop %rdx
    pop %rcx
    ret


# iotahex:#rdi <-- num, rsi <-- str_dst 
#     # '0' == 48 == 0x30
#     push %rcx
#     movl $16, %ecx
#     push %rdx
#     push %rax
#     mov %rdi, %rax #cur_num <-- rdi

#     dec %rsi #str_dst--
#     movb $0, (%rsi) #null terminate the digits string.

#     atoi_check_digit:
#     cmp $0, %rax
#     je atoi_finish

#     # divide quad_num by 10, put the remainder char in str_dst and increment the dst:
#     xor %rdx, %rdx
#     div %ecx #cur_num <-- cur_num/10
#     addl $48, %edx #edx <-- remainder in ascii
#     dec %rsi #str_dst--
#     movb %dl, (%rsi)
#     jmp atoi_check_digit
    
#     atoi_finish:
#     pop %rax
#     pop %rdx
#     pop %rcx
#     ret

printdec:#%rdi <-- num
    push %rax
    push %rsi
    push %rdi

    call count_dec_digits

    #insert null terminted digit string to the buffer:
    movq %rsp, %rsi #the rsi argument of atoi.
    #create buffer for digit string:
    sub %rax, %rsp
    dec %rsp
    call iotadec
    
    #print the digit string:
    movq %rsp, %rdi
    call printstr

    #remove buffer:
    inc %rsp
    add %rax, %rsp

    #print endline:
    mov $endl_msg, %rdi
    call printstr

    pop %rdi
    pop %rsi
    pop %rax
    ret

bar:
    

foo:
    cmp $0, %rdi
    je foo_end

    push %rax
    push %rdi

    dec %rdi
    call foo

    movq $1, %rax
    movq $1, %rdi
    movq $foo_msg, %rsi
    movq $1, %rdx
    syscall
    
    movq $1, %rax
    movq $1, %rdi
    movq $0, %rsi
    movq $1, %rdx
    syscall

    movq %rax, %rdi
    call printdec

    pop %rdi
    pop %rax
    foo_end:
    ret

_start:
    movq $5, %rdi
    call foo

    movq $endl_msg, %rdi
    call printstr
    movq $endl_msg, %rdi
    call printstr

    movq $2, %rdi
    call foo

    movq $60, %rax
    movq $0, %rdi
    syscall
