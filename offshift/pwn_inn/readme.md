# The pwn inn
#### Description
As we know that crypto is a hot potato right now, we wanted to welcome you to a safe place, The Pwn Inn. We've had many famous faces stay in our Inn, with gets() and printf() rating us 5 stars. We've decided to start making an app, and wanted you guys to be our beta testers! Welcome!
#### Author
Tango
#### Points and solves
477 points, 58 solves.

In this challenge we are given a binary which calls a function ```vuln```
```c
void vuln(void)
{
  long in_FS_OFFSET;
  char local_118 [264];
  undefined8 local_10;
  
  local_10 = *(undefined8 *)(in_FS_OFFSET + 0x28);
  fgets(local_118,0x100,stdin);
  printf("Welcome ");
  printf(local_118);
  exit(1);
}```

We simply have a classic printf format string attack.
We are not given a libc version, so we have to leak libc version aswell.


## 1. Patching ```exit(1)```
We'll, main doesn't return, instead it calls ```exit(1)```, and in order to be able to leak, and ret2libc we need to have atleast two rounds of format string attack.
So we have to patch ```exit(1)``` to something else, in this case it is easy and convenient to patch it with ```vuln```.
This way, every time vuln finishes, it calls itself and we have endless rounds of format string attacks that we can perform.

```python
p.sendlineafter("? \n", b"%45$08p%46$08p%4199080d%10$nAAAA" + p64(elf.sym.got["exit"]) + 8*b"B")
```

Let's breakdown the payload step by step.
#### a. ```%45$08p%46$08p```
Simply translating the format string:
"Print the 45th argument to printf as a pointer padded to size of 8 chars."
The same follows for the 46th argument.
Now, we ask ourselves what does the 45th and 46th arguments hold? Two halves of the return address of main.
Main returns to __libc_start_main, and by leaking the return address of the main function we can narrow down the libc version of the remote to a few libc versions.

### b. ```%4199080d```
Yo ```hex(4199080) = 4012A8```, This means that printf will print its' next argument as a decimal, padded by ```4199080``` digits.
Notice that the ```vuln``` is at: ```0x4012c4```.
We already printed 16 characters by leaking the 45th and 46th arguments which totals to ```0x4012c``` printed characters.
This allows us to easily overwrite ```exit``` to ```vuln``` using the next step.

### c. ```%10$nAAAA + p64(elf.sym.got["exit"]) + 8*b"B"```
Remind ourselves of the ```%n``` format modifier of printf, which writes the total characters written so far to the value to the pointed by the next argument of printf.
Or if we specifiy ```%idx$n```, it writes that value to the value pointed by the ```idx```th argument to printf.
So, with a bit of debugging, it's possible to align the GOT entry for ```exit``` to be the ```10th``` argument to printf. (Using the ```A```s to push the address around the stack, and ```B```s simply for visibility.)
And by doing that we write ```vuln = 0x4012c4``` to the GOT of ```exit```.

## 2. Getting shell
So, now, every time vuln exits, it actually calls itself and after sending the first paylod we enter a second round of format string attack.
Moreover, we leaked a libc address from ```__libc_start_main``` and we can narrow down the libc version and calculate the offset of ```system```.
Now the plan is, to overwrite the GOT entry of  ```printf``` to ```system```.
However, the problem is that the address of system is so large, that in if we want to print that many characters the challenge will time us out.
We must split and write in three small batches. (Also possible in two batches)

Lets visualize the address of ```system```.
For example lets say that ```system```` is at ```0xAAAABBBBCCCCDDDD``` in our case ```0xAAAA = 0x0000```
So, let's assume that ```BBBB``` > ```CCCC``` > ```DDDD```. 
We can write ```0xDDDD``` characters using printf, and immediatly after that overwrite the first 2 bytes of ```GOT.printf``` with ```DDDD```
After that, we can print ```0xCCCC-0xDDDD``` bytes, which totals to ```0xCCCC``` characters printed overall and overwrite bytes 3 and 4 of ```GOT.printf```.
Similiarly we can continue to ```BBBB```.
However that assumption doesnt always hold, because libc addresses are randomized, so we have to get lucky for our assumption to hold.
But sorting ```[BBBB, CCCC, DDDD]``` and constructing the format string from the lowest entry to the highest will make the attack successfull every time.

The final format string attack:
```l = [third1, third2, third3]
l.sort()
payload = b"%" + ("%05d"%l[0]).encode("ascii") + b"d%11$hn" +\
	b"%" + ("%05d"%(l[1]-l[0])).encode("ascii") + b"d%12$hn" +\
	b"%" + ("%05d"%(l[2]-(l[1]))).encode("ascii") + b"d%13$hn" +1*b"\x00"
for i in range(0,3):
    if l[i] == third1:
        payload += p64(elf.sym.got["printf"]+4)
    if l[i] == third2:
        payload += p64(elf.sym.got["printf"]+2)
    if l[i] == third3:
        payload += p64(elf.sym.got["printf"])
```

Seems complex, but if you have been following the explanation it's straight forward.
```l``` is the list ```[BBBB, CCCC, DDDD]```, we sort it.
Now we can construct the payload and print 3 batches of characters in order from lowest to highest.
Each batch will print the correct amount of characters and afterwards will perform ```d%11$hn``` (or ```%12$n``` or ```%13$n```). Which will write a short to the appropriate address. (because we use ```hn``` instead of ```n```).

Last part of the payload that is constructed in the for-loop simply matches the correct offset of GOT.printf address with the right batch.
For example, if assume for the sake of explanation ```third1 < third2 < third3```. 
The first batch of the payload will print ```third1``` characters and will write the result to the 11th argument. Therefore the 11th argument must correspond to the offset that is appropriate for writing the first third.
The same follows for ```third2``` and ```third3```.
In parallel to the example: ```0xAAAABBBBCCCCDDDD```, ```AAAA``` belongs in ```GOT.printf + 6```, ```BBBB```` belongs in ```GOT.printf + 4``` and so on.

We send this line, and then ```exit``` returns us to ```vuln```, write ```/bin/sh``` and this parameter will be passed to printf which is now actually ```system``` :)


### How to narrow libc version
A very nice website at [libc database search](https://libc.nullbyte.cat/) will narrow down the libc version based on known offsets.




