#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>


#define HEAP_SIZE (0x1000)

typedef struct __attribute__((packed)) chunk_s  {
    struct chunk_s * next;
    int free;
    int size;
} chunk_t;

void * communist_heap = NULL;

int get_int()
{
    char buffer[10];
    fgets(buffer, sizeof buffer, stdin);
    return atoi(buffer);
}

void * get_free_chunk(int size)
{
    chunk_t * chunk_ptr = (chunk_t *)communist_heap;
    while ((char*)chunk_ptr < (char*)communist_heap+HEAP_SIZE)
    {
        if ((chunk_ptr->size == 0) || (chunk_ptr->free && chunk_ptr->size >= size + sizeof(chunk_t)))
        {
            return chunk_ptr;
        }

        chunk_ptr = chunk_ptr->next;
    }

    return NULL;
    
}


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

void heapify(chunk_t * chunk, size_t new_size)
{
    if (new_size == chunk->size)
    {
        return;
    }

    ((chunk_t *)((char *)chunk + new_size + sizeof(chunk_t)))->size = chunk->size - new_size;
    ((chunk_t *)((char *)chunk + new_size + sizeof(chunk_t)))->free = 1;
    chunk->size = new_size;
    chunk->next = (chunk_t *)((char *)chunk + new_size + sizeof(chunk_t));
    chunk->free = 0;
}

void allocate_heap()
{
    puts("Comrade, what size?");
    int size = get_int();
    void * available_chunk = NULL;
    if (size > HEAP_SIZE) 
    {
        puts("Gulag alert!");
        exit(1);
    }

    available_chunk = get_free_chunk(size);
    if (available_chunk == NULL)
    {
        goto cleanup;
    }

    heapify(available_chunk, size);

    puts("Comrade, what data?");
    fgets((char*)(available_chunk) + sizeof(chunk_t), size, stdin);
    ((char*)(available_chunk) + sizeof(chunk_t))[strlen(((char*)(available_chunk) + sizeof(chunk_t)))] = 0;

cleanup:    

    return;
}

void show_heap()
{
    puts("Comrade, what index?");
    int index = get_int();
    char * heap_data = (char*)get_chunk_at_index(index) + sizeof(chunk_t);
    puts(heap_data);
}

void free_heap()
{   
    int index = get_int();
    chunk_t * chunk = get_chunk_at_index(index);
    chunk->free = 1;
}

void steal_heap()
{
    puts("Where does your neighbor live at?");
    int source = get_int();
    puts("Where do you live at?");
    int destination = get_int();
    
    puts("How much to steal?");
    int amount = get_int();
    chunk_t * source_chunk = get_chunk_at_index(source);
    chunk_t * destination_chunk = get_chunk_at_index(destination);
    if (amount > source_chunk->size)
    {
        puts("Gulag alert");
        exit(1);
    }

    source_chunk->size -= amount;
    destination_chunk->size += amount;
}

void edit_heap()
{
    puts("Comrade, heap to edit?");
    int index = get_int();
    puts("Comrade, new content of the heap?");
    chunk_t * chunk = get_chunk_at_index(index);

    fgets((char*)(chunk) + sizeof(chunk_t), chunk->size, stdin);
    ((char*)(chunk) + sizeof(chunk_t))[strlen((char*)(chunk) + sizeof(chunk_t))] = 0;
}

void setup_communist_heap()
{
    puts("Welcome to russia!");
    puts("Here at the gulag, we invented the new best heap; the communist heap!");
    puts("This heap is communist, if you need more space; you can steal from your neighbors");
    puts("Can you make putin proud?");

    communist_heap = mmap(NULL, HEAP_SIZE, 7, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memset(communist_heap, 0, HEAP_SIZE);
    ((chunk_t *)communist_heap)->next = NULL;
    ((chunk_t *)communist_heap)->size = HEAP_SIZE - sizeof(chunk_t);
    ((chunk_t *)communist_heap)->free = 1;
}

void show_menu()
{
    puts("1. Communist allocate");
    puts("2. Communist free");
    puts("3. Communist steal");
    puts("4. Communist edit");
    puts("5. Communist show");
    puts("Comrade choice?");
}

int main()
{
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
    setbuf(stderr, NULL);
    setup_communist_heap();
    int choice = -1;
    while (1)
    {
        show_menu();
        choice = get_int();
        if (choice == 1)
        {
            allocate_heap();
        }
        else if (choice == 2)
        {
            free_heap();
        }
        else if (choice == 3)
        {
            steal_heap();
        }
        else if (choice == 4)
        {
            edit_heap();
        }
        else if (choice == 5)
        {
            show_heap();
        }
    }

    return 0;
}