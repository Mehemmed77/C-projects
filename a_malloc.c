#include <stdio.h>
#include <unistd.h>

struct block {
    struct block *next;
    size_t size; // excluding metadata
    unsigned int free: 1;
    char symbol;
};

typedef struct block block;

block* head = NULL;
block* tail = NULL;

char* HEAP_START = NULL;
char* HEAP_END = NULL;
char* PTR = NULL;

const int ALIGN_AMT = 8;
const int INC_AMT = 4096;
const unsigned int SPLIT_THR = 128;

void print_heap() {
    block* curr = head;
    char* user_memory_address;

    while(curr != NULL) {
        user_memory_address = ((char*) curr) + sizeof(block);
        printf("============\nSymbol:%c\nBlock address:%p\nUser memory address:%p\nNext block address:%p\nSIZE: %zu\nFree:%u\n\n", 
            curr->symbol, curr, user_memory_address, curr->next, curr->size, curr->free);
        curr = curr->next;
    }
}

void insert_new_block(void* ptr, size_t size, char symbol) {
    block* new_block = (block*) ptr;
    new_block->free = 0;
    new_block->size = size;
    new_block->next = NULL;
    new_block->symbol = symbol;

    if (head == NULL) head = new_block;

    if (tail != NULL) tail->next = new_block;

    tail = new_block;
}

block* find_first_fit(size_t size) {
    block* curr = head;

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
    HEAP_START = sbrk(0);
    sbrk(INC_AMT);
    HEAP_END = HEAP_START + INC_AMT;
    PTR = HEAP_START;
}

size_t align8(size_t size) {
    if (size % ALIGN_AMT == 0) return size;

    return ((size / ALIGN_AMT) + 1) * 8; 
}

void* a_malloc(size_t size, char symbol) {
    if (HEAP_START == NULL) init();

    size = align8(size);

    int available_space = (int) (HEAP_END - PTR);

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
        HEAP_END += INC_AMT;
        available_space = HEAP_END - PTR;
    }

    insert_new_block(PTR, size, symbol);

    void *curr = (void *) (PTR + sizeof(block));
    PTR += sizeof(block) + size;

    return curr;
}

void a_free(void* ptr) {
    block* block_ptr = (block *) ((char *) ptr - sizeof(block));

    block_ptr->free = 1;
}

int main() {
    char* a = a_malloc(300, 'A');

    print_heap();

    printf("\n=== FREE A ===\n\n");
    a_free(a);

    print_heap();

    printf("\n=== ALLOCATE SMALLER BLOCK ===\n\n");
    char* b = a_malloc(50, 'B');

    print_heap();

    return 0;
}