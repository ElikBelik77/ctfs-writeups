section .data
rop_rapper db "ROP RAPPER GOES SKRRRRRRRRRRRRRRRRRR", 0xa, 0xd
rop_rapper_1 db "WHERE DO YOU WANT THE ROP RAPPER TO SPIT BARS", 0xa, 0xd
rop_rapper_2 db "WHAT BARS WILL YO DAWG DROP?", 0xa
SKRRRRRR db "SKRRRRRRRRRRRRRR"


in_offset db 0
in_qword times 8 db 0
section .text
global main
main:
	mov rax, 1
	mov rdi, 1
	mov rsi, rop_rapper
	mov rdx, 0x25
	syscall
	mov rax, 1
	mov rsi, rop_rapper_1
	mov rdx, 0x2f
	syscall
	mov rax, 0
	mov rdi, 0
	mov rsi, in_offset
	mov rdx, 2
	syscall
	mov rax, 1
	mov rdi, 1
	mov rsi, rop_rapper_2
	mov rdx, 0x1d
	syscall
	mov rax, 0
	mov rdi, 0
	mov rsi, in_qword
	mov rdx, 8
	syscall
	xor rdi, rdi
	xor rsi, rsi
	movzx rdi, BYTE [in_offset]
	add rdi, main
	mov rsi, QWORD [in_qword]
	mov [rdi], rsi
	ret
	call main
	times 90 nop
