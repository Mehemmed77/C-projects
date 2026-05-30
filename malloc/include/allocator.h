#include <stdio.h>

void* a_malloc(size_t size, char symbol);
void a_free(void* ptr);

void print_heap(void);
void coalesce_blocks(void);

struct block {
    struct block *next;
    size_t size;
    unsigned int free: 1;
    char symbol;
};

typedef struct block block;
