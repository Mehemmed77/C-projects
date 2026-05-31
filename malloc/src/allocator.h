#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdio.h>

void* a_malloc(size_t size, char symbol);
void a_free(void* ptr);

void print_heap(void);
void coalesce_blocks(void);

#endif