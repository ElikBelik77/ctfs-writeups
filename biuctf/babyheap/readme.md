# Babyheap

## Description

**Author: *ElikBelik77***

> Even babies can play with heaps, can you win the baby?

### Points and solves

500 points, and 1 solve.

## Solution

This challenge exposes a simple vulnerability of tcache duplication (I suggest you read about heap exploitation online if you are unfamiliar with tcache duplication).

The version of libc was not provided, however by leaking offsets of known functions in libc, it was possible to deduce that the libc version on the remote was 2.27.

Given that we are dealing with libc 2.27, and that the binary allows double frees:

```c

if (index >= 0 && index <= 7) {
        free(heap[index]);
        sizes[index] = 0;

}
```

We can abuse tcache duplication exploit by sending the following payload;
> Code snippet from exploit.py

```python
add_note(0, 0x20, 8*b"A")
delete_note(0)
delete_note(0)
```

This gives us write what where bug, which allows us to change the GOT entry of exit to be the address of the win function. 