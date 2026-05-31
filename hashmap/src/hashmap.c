#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define FNV_32_PRIME ((uint32_t)0x01000193)
#define FNV_32_OFFSET ((uint32_t)0x811C9DC5)
#define INITIAL_CAPACITY 2

typedef struct bucket_t {
    char* key;
    void* value;
    struct bucket_t* next;
} bucket;

typedef struct hashmap_t {
    int capacity;
    int size;
    bucket** buckets;
} hashmap;

char* copy_str(char* str) {
    size_t len = strlen(str) + 1;

    char* new_str = malloc(len);

    for(int i = 0; i < len; i++) {
        *(new_str + i) = *(str + i);
    }

    return new_str;
}

uint32_t fnv1a_32_str(char* key)  {
    uint32_t hash = FNV_32_OFFSET;

    while(*key) {
        hash ^= (uint32_t) (unsigned char) (*key);
        hash *= FNV_32_PRIME;
        key++;
    }

    return hash;
}

hashmap* hashmap_create() {
    hashmap* map = malloc(sizeof(hashmap));

    map->capacity = INITIAL_CAPACITY;
    map->size = 0;

    map->buckets = calloc(INITIAL_CAPACITY, sizeof(bucket*));

    return map;
}

bool key_equality(char* key1, char* key2) {
    while(*key1 && *key2) {
        if(*key1 != *key2) return false;

        key1++;
        key2++;
    }

    return !(*key1 || *key2);
}

bucket* init_bucket(char* key, void* value) {
    bucket* b = malloc(sizeof(bucket));
    b->key = copy_str(key);
    b->value = value;
    b->next = NULL;

    return b;
}

void hashmap_put(hashmap* map, char* key, void* value) {
    uint32_t hash = fnv1a_32_str(key);
    size_t idx = hash % map->capacity;

    bucket** buckets = map->buckets;

    bucket* b = *(buckets + idx);

    if (b == NULL) {
        buckets[idx] = init_bucket(key, value);
        return;
    }

    bucket* prev = NULL;
    
    while(b != NULL) {
        if(key_equality(b->key, key)) {
            b->value = value;
            return;
        };

        prev = b;
        b = b->next;
    }

    prev->next = init_bucket(key, value);
}

int main() {
    hashmap* map = hashmap_create();

    int values[10];

    for (int i = 0; i < 10; i++) {
        values[i] = i;

        char key[16];
        sprintf(key, "key%d", i);

        hashmap_put(map, key, &values[i]);
    }

    for (int i = 0; i < map->capacity; i++) {
        printf("=== bucket[%d] ===\n", i);

        bucket* curr = map->buckets[i];

        while (curr) {
            printf(
                "key=%s value=%d next=%p\n",
                curr->key,
                *(int*)curr->value,
                curr->next
            );

            curr = curr->next;
        }
    }

    return 0;
}