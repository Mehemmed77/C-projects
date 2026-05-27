#include <stdio.h>
#include <unistd.h>

struct block {
    struct block *next;
    size_t size; // excluding metadata
    unsigned int free: 1;
};

typedef struct block block;

block tail;

char* HEAP_START = NULL;
char* HEAP_END = NULL;
char* PTR = NULL;

const int ALIGN_AMT = 8;
const int INC_AMT = 4096;

void init() {
    HEAP_START = sbrk(0);
    sbrk(INC_AMT);
    HEAP_END = HEAP_START + INC_AMT;
    PTR = HEAP_START;
}

size_t align8(size_t size) {
    if (size % ALIGN_AMT == 0) return size;

    return (size / ALIGN_AMT) * ALIGN_AMT + (size - size % ALIGN_AMT); 
}

void* a_malloc(size_t size) {
    if (HEAP_START == NULL) init();

    size = align8(size);

    int available_space = (int) (HEAP_END - PTR);
    
    while(available_space < size) {
        sbrk(INC_AMT);
        HEAP_END += INC_AMT;
        available_space = HEAP_END - PTR;
    }

    if (tail.size == 0) { // not initialized
        
    }

    block new_block = {  };

    int *curr = (int *) PTR;
    PTR += size;
    return curr;
}

int main() {
    int* a = (int *) a_malloc(10);
    int* b = (int *) a_malloc(20);
    int* c = (int *) a_malloc(20);

    printf("%p\n%p\n%p\n", a, b, c);

    return 0;
}