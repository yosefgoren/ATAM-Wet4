.global _start, foo

.section .text
foo:
mov    $0x2,%rax
mov    $0x6000dd,%rdi
mov    $0x0,%rsi
jmp outer_syscall
rett:
dec %r13
mov    $0x2,%rax
mov    $0x6000dd,%rdi
mov    $0x0,%rsi
pushq  %r13
syscall 
pop %r13
retq   

_start:
movq $1, %r13
callq  foo
outer_syscall:
syscall
cmp $0, %r13
jne rett
mov    $0x3c,%rax
mov    $0x0,%rdi
syscall 
xor    %rax,%rax
