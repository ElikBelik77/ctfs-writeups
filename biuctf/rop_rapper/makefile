all:
    nasm -f elf64 shellcode.asm -o shellcode.o
	objcopy shellcode.o --dump-section .text=shellcode_raw