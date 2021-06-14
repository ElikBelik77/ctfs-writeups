# Communism Heap

## Description

**Authors: *ElikBelik77***

> I heard from my friend in russia, that he has a new heap allocator called communism heap, can you show him that communism can't work?

You don't need the libc version, you can leak the libc version yourself.

### Points and solves

500 points, and 0 solves.

## Solution

This challenge introduces a new allocation method; the communist method. The communist method defines a chunk:

```c
typedef struct __attribute__((packed)) chunk_s  {
    struct chunk_s * next;
    int free;
    int size;
} chunk_t;
```

Each chunk contains a pointer to the next chunk, whether the chunk is free, and the size of the data in the chunk.

The wrapper for the allocation system exposes 5 operations:

- allocation
- deallocation
- editing contents of a chunk
- stealing
- displaying the contents of a chunk

The allocation is quite simple, it is a First-Fit alogirthm which traverses the linked list of chunks and inserts the chunk into the appropriate position.
Deallocation simply turns off the ```free``` field.
Editing contents of a chunk prompts the user for ```size``` bytes and changes the contents of a selected chunk by index.
Stealing, prompts the user for two indexes and ```amount```, and will transfer ```amount``` bytes from one chunk's ```size``` field to the other chunk ```size``` field.

### The exploit

Snippet from the source:

```c
chunk_t * get_chunk_at_index(int index)
{
    chunk_t * ptr = (chunk_t *)communist_heap;
    for (int i = 0; i < index; i++)
    {
        if ((void*)ptr > (void*)((char*)communist_heap + HEAP_SIZE))
        {
            return NULL;
        }
        
        ptr = ptr->next;
    }

    return ptr;
}
```

This function is used everywhere when attempting to access a chunk at a given index. The problem here is that the entire tranversal is done using the linked list, and is not validated against the ```size``` field.

Using the stealing functionallity, we can overflow a buffer in a way that will override the ```next``` field of the chunk that is ahead.

Have a look at an example of two chunks that have been allocated one after the other:
</br>

| *next | free = 0 | size = 50 | 50 bytes of data | *next | free = 0 | size = 50 | 50 bytes fo data |

Now, using the steal function, I will steal 20 bytes from the second chunk into the first chunk.
</br>

| *next | free = 0 | size = 70 | 50 bytes of data | *next | free = 0 | size = 30 | 30 bytes of data | 20 bytes of data |

Now if I will edit the first chunk, the program will allow me to edit 70 bytes of data; any byte after the 50th byte will override the ```chunk_t``` header of the next chunk.

This means, that if I override the ```next``` pointer of the second chunk to be an address of a GOT entry, and request to view the contents of the third chunk, I will actually leak the address of that function in libc.

This way we can deduce the version of libc, and moreover using ```edit``` I can change the address that is stored in the GOT.

The provided solution leaks the address of ```puts```, calculate the offset of system based on the libc version and override ```atoi``` to be system.

Now the next time that I input a number, that number is actually passed to ```system```. My favorite number is ```/bin/sh``` :)

## A snippet from the solution

This creates a healthy chain of allocations, the intentions is to overflow the second chunk, in order to cause the ```next``` pointer of thrid chunk to be a GOT function.
The last chunk is big, just to have enough bytes to steal.

```python
allocate(0x20, "PWNED")
allocate(8, "AAAAAA")
allocate(0x10, "BBBBBB")
allocate(0x10, "CCCCCC")
allocate(0x100, "DDDDDD")
```

Here I'm stealing bytes from the last chunk and moving them into the second chunk.
Afterwards I'm overflowing the second chunk data and modifying the next pointer of the third chunk to be 16 bytes behind the GOT of ```puts```.
I modify it to be 16 bytes behind because when calling ```show(3)``` I'm displaying the data of the chunk:

```python
steal(4, 1, 0x9F)
edit(1, b"a" * 0x8 + p64(PUTS_GOT - 0x10))
show(3)
```

It is clear that the function skips the header of the chunk by adding ```sizeof(chunk_t)``` to the chunk offset. So it happens that ```sizeof(chunk_t) = 0x10```. This way when displaying the third chunk I'm actually leaking the address of ```puts``` (which is the 'data' of the third chunk)

Now I know where system is and can change the address of ```atoi```.

```c
void show_heap()
{
    puts("Comrade, what index?");
    int index = get_int();
    char * heap_data = (char*)get_chunk_at_index(index) + sizeof(chunk_t);
    puts(heap_data);
}
```

Same as before, modiying the ```next``` pointer of the third chunk to be positioned appropriately in relation to the GOT address of ```atoi``` and modifying it to be ```system```.

The next time I enter an interger, which is at the next choice selection, ```system``` will be called instead of ```atoi```.

```python
edit(1, b"a" * 0x8 + p64(ATOI_GOT - 0x10))
edit(3, p64(system_address))
```
