#include <stdio.h>
#include <unistd.h>

struct block {
    struct block *next;
    size_t size; // excluding metadata
    unsigned int free: 1;
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
    block* temp = head;
    char* user_memory_address;

    while(temp != NULL) {
        user_memory_address = ((char*) temp) + sizeof(block);
        printf("============\nBlock address:%p\nUser memory address:%p\nNext block address:%p\nSIZE: %zu\nFree:%u\n\n", 
            temp, user_memory_address, temp->next, temp->size, temp->free);
        temp = temp->next;
    }
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

void* a_malloc(size_t size) {
    if (HEAP_START == NULL) init();

    size = align8(size);

    int available_space = (int) (HEAP_END - PTR);

    while(available_space < size + sizeof(block)) {
        sbrk(INC_AMT);
        HEAP_END += INC_AMT;
        available_space = HEAP_END - PTR;
    }

    block* temp = (block*) PTR;
    temp->free = 0;
    temp->size = size;
    temp->next = NULL;

    if (head == NULL) head = temp;

    if (tail != NULL) tail->next = temp;

    tail = temp;

    void *curr = (void *) (PTR + sizeof(block));
    PTR += sizeof(block) + size;

    return curr;
}

void a_free(void* ptr) {
    block* block_ptr = (block *) ((char *) ptr - sizeof(block));
    
    block_ptr->free = 1;
}

int main() {
    int* a = (int *) a_malloc(10);
    int* b = (int *) a_malloc(20);
    int* c = (int *) a_malloc(20);
    int* d = (int *) a_malloc(20);
    
    a_free(c);

    print_heap();

    return 0;
}