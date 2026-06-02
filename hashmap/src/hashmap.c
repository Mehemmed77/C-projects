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

void print_str(char* str) {
    while(*str) {
        printf("%c", *str);
        str++;
    }
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

insert_existing_bucket(
    bucket** buckets,
    bucket* existing_bucket,
    int capacity
)
{
    uint32_t hash = fnv1a_32_str(existing_bucket->key);
    size_t idx = hash % capacity;

    bucket* b = buckets[idx];

    if (b == NULL) {
        buckets[idx] = existing_bucket;
        return;
    }

    bucket* prev = NULL;
    while(b != NULL) {
        prev = b;
        b = b-> next;
    }

    prev->next = existing_bucket;
}

bool insert_new_bucket(
    bucket** buckets,
    char* key,
    void* value,
    int capacity
) {
    // returns true if new key was added, otherwise false
    uint32_t hash = fnv1a_32_str(key);
    size_t idx = hash % capacity;

    bucket* b = buckets[idx];

    if (b == NULL) {
        buckets[idx] = init_bucket(key, value);
        return true;
    }

    bucket* prev = NULL;
    while(b != NULL) {
        if (key_equality(b->key, key)) {
            b->value = value;
            return false;
        }

        prev = b;
        b = b-> next;
    }

    prev->next = init_bucket(key, value);
    return true;
}

// void free_buckets(bucket** buckets, int capacity) {
//     for(int idx = 0; idx < capacity; idx++) {
//         bucket* b = buckets[idx];

//         bucket* temp = b;
//         while(temp != NULL) {
//             temp = b->next;
//             free(b->key);
//             free(b);
//             b = temp;
//         }
//     }

//     free(buckets);
// }

bucket** resize(hashmap* map) {
    int old_capacity = map->capacity;

    bucket** old_buckets = map->buckets;
    bucket** new_buckets = calloc(old_capacity * 2, sizeof(bucket*));

    for(int idx = 0; idx < old_capacity; idx++) {
        bucket* b = old_buckets[idx];

        bucket* next;

        while(b != NULL) {
            next = b->next; 
            b->next;
            insert_existing_bucket(new_buckets, b, old_capacity * 2);
            b = next;
        }
    }

    return new_buckets;
}

void hashmap_put(hashmap* map, char* key, void* value) {
    float load_factor = (float) map->size / map->capacity;

    if (load_factor > 0.75) {
        bucket** new_buckets = resize(map);
        // free_buckets(map->buckets, map->capacity);
        map->capacity *= 2;
        map->buckets = new_buckets;
    }

    bool new_key_was_added = insert_new_bucket(map->buckets, key, value, map->capacity);

    if(new_key_was_added) map->size++; 
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