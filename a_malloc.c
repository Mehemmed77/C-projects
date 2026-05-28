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

char* find_first_fit(size_t size, char symbol) {
    block* curr = head;

    while(curr != NULL) {
        if (curr->free && curr->size >= size) {
            curr->symbol = symbol;
            curr->free = 0;
            return (char*) curr + sizeof(block);
        }

        curr = curr->next;
    }

    return NULL;
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

    char* addr = find_first_fit(size, symbol);

    if(addr != NULL) return addr;

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
    int* a = (int *) a_malloc(10, 'a');
    int* b = (int *) a_malloc(20, 'b');
    int* c = (int *) a_malloc(20, 'c');
    
    print_heap();
    a_free(c);

    int* d = (int *) a_malloc(20, 'd');

    print_heap();

    return 0;
}