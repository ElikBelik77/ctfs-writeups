## Faking till you're making:
We are given a small program
```c
int main(void)
{
  char *__s;
  ulonglong data [10];
  char *b;
  ulonglong *a;
  setvbuf(stdout,(char *)0x0,2,0);
  printf("%p\n",sh);
  malloc(1);
  read(0,data,0x50);
  free(data + 2);
  __s = (char *)malloc(0x30);
  fgets(__s,0x404,stdin);
  return 0;
}
```
and a win function:
```c
void sh(void)
{
  system("/bin/sh");
  return;
}
```

# The vulnerability:
It is very clear that this binary is vulnerable to the House of Spirits;
That is because we have full control ```data``` and we free ```data + 0x2```
So we can forge a fake chunk of size with size ```0x40``` and when we free it, it'll end up on the tcache of size 0x40.
When we malloc ```__s``` the glibc heap will reuse the tcahce block, which is on the stack.
We read into ```__s``` which lets us overflow the jump into the win function.

# The chunk:
```fake_chunk1 = p64(0) + p64(0x40) + 6*p64(0)```
We set the size to 0x40, with PREV_INUSE = 0, and this is enough to trick glibc into thinking this is a legitimate chunk to free.
After we send this chunk, it is a simple buffer overflow into the win function.
