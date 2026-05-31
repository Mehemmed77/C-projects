#include <stdio.h>
#include <unistd.h>
#include "allocator.h"

struct block {
    struct block *next;
    size_t size; // excluding metadata
    unsigned int free: 1;
    char symbol;
};

typedef struct block block;

typedef struct {
    block* head;
    block* tail;

    char* HEAP_START;
    char* HEAP_END;
    char* PTR;
} allocator_t;

static allocator_t allocator;

enum {
    ALIGN_AMT = 8,
    INC_AMT = 4096,
    SPLIT_THR = 128
};

void print_heap() {
    block* curr = allocator.head;
    char* user_memory_address;

    while(curr != NULL) {
        user_memory_address = ((char*) curr) + sizeof(block);
        printf("============\nSymbol:%c\nBlock address:%p\nUser memory address:%p\nNext block address:%p\nSIZE: %zu\nFree:%u\n\n", 
            curr->symbol, curr, user_memory_address, curr->next, curr->size, curr->free);
        curr = curr->next;
    }
}

void coalesce_blocks() {
    if (allocator.head == NULL) return;

    block* curr = allocator.head;
    block* next = curr->next;

    while(curr && next) {
        if (curr->free && next->free) {
            curr->size += sizeof(block) + next->size;
            curr->next = next->next;
        }

        curr = curr->next;
        if (curr) next = curr->next;
    }
}

void insert_new_block(void* ptr, size_t size, char symbol) {
    block* new_block = (block*) ptr;
    new_block->free = 0;
    new_block->size = size;
    new_block->next = NULL;
    new_block->symbol = symbol;

    if (allocator.head == NULL) allocator.head = new_block;

    if (allocator.tail != NULL) allocator.tail->next = new_block;

    allocator.tail = new_block;
}

block* find_first_fit(size_t size) {
    block* curr = allocator.head;

    while(curr != NULL) {
        if (curr->free && curr->size >= size) return curr;

        curr = curr->next;
    }

    return NULL;
}

void split_block(block* b, size_t size) {
    block* prev_next = b->next;

    block* new_b = (block*) ((char*) (b + 1) + size);

    new_b->free = 1;
    new_b->size = b->size - size - sizeof(block);
    new_b->next = prev_next;
    new_b->symbol = '?';

    b->next = new_b;
    b->size = size;
}

void init() {
    allocator.HEAP_START = sbrk(0);
    sbrk(INC_AMT);
    allocator.HEAP_END = allocator.HEAP_START + INC_AMT;
    allocator.PTR = allocator.HEAP_START;
}

size_t align8(size_t size) {
    if (size % ALIGN_AMT == 0) return size;

    return ((size / ALIGN_AMT) + 1) * 8; 
}

void* a_malloc(size_t size, char symbol) {
    if (allocator.HEAP_START == NULL) init();

    size = align8(size);

    int available_space = (int) (allocator.HEAP_END - allocator.PTR);

    block* b = find_first_fit(size);

    if(b != NULL) {
        if(b->size - size - sizeof(block) >= SPLIT_THR) {
            split_block(b, size);
        };

        b->free = 0;
        b->symbol = symbol;

        return (void *) (b + 1);
    }

    while(available_space < size + sizeof(block)) {
        sbrk(INC_AMT);
        allocator.HEAP_END += INC_AMT;
        available_space = allocator.HEAP_END - allocator.PTR;
    }

    insert_new_block(allocator.PTR, size, symbol);

    void *curr = (void *) (allocator.PTR + sizeof(block));
    allocator.PTR += sizeof(block) + size;

    return curr;
}

void a_free(void* ptr) {
    block* block_ptr = (block *) ((char *) ptr - sizeof(block));

    block_ptr->free = 1;
}