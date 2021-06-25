.global _start, foo


.section .text

_start:
	mov $10, %r13
	call foo
	mov $60, %rax
	mov $0, %rdi
	syscall


foo:
	mov $-1, %rax
	syscall
	dec %r13
	call goo
	cmp $0, %r13
	je rett

goo:
	mov $-1, %rax
	syscall
	dec %r13
	cmp $0, %r13
	je rett
	call foo

rett:
	ret

