#include "allocator.h"
#include <stdio.h>

int main() {
    char* a = a_malloc(100, 'A');
    char* b = a_malloc(100, 'B');
    char* c = a_malloc(100, 'C');
    char* d = a_malloc(100, 'D');

    a_free(b);
    a_free(c);

    printf("=== BEFORE COALESCING ===\n");
    print_heap();

    coalesce_blocks();

    printf("=== AFTER COALESCING ===\n");
    print_heap();

    return 0;
}