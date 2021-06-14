# Rop Rapper

## Description
**Author: *ElikBelik77***

> THE ROP RAPPER GOES SKRRRRRRRRRRRRRRRR
</br>
WILL YOU HELP MY HOMIE DROPPING SOME SICK BARZ?
</br>
HE'LL GIVE YOU THE FLAG IN RETURN

The flag is inside a file called "flag.txt"

### Points and solves

500 points, and 1 solve.

## Solution

The binary is a simple assembly file, that uses syscalls to print and read, and the text segment is RWX. (Has read, write and execute permissions)

The rop rapper asks the user for an offset and an 8 byte value, that value will be written in the text section at the given offset.

To solve this challenge, you first have to override the return statement into a call main statement:

```asm
    mov [rdi], rsi
    ret
    call main
```

I conviniently placed a call main instruction at the end of the code, just so it will be easier for the players to mimic a call instruction.

After patching the return into a call, we can insert an ```execve("/bin/sh", ["/bin/sh"], NULL)``` shellcode into the end of the binary, which I nicely padded with 90 ```NOP```s

After writing the entire shellcode, simply turn the ```call main``` instruction into 8 ```NOP```s and finish the challenge.

This challenge was quite trivial so I didn't add a full solution, however I wrote an example shellcode and a nice way to build it. In your pwntools script you can use: 

```python
shellcode = open("shellcode_raw", "rb").read()
```
