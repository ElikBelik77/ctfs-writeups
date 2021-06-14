section .text
global _start

start:

call get_bin_sh_string
db "/bin/sh", 00
get_bin_sh_string:

xor     rax, rax
xor     rdx, rdx
pop     rdi
push    rax
push    rdi
mov     rsi, rsp

mov     al, 0x3b
syscall