#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
// ================== TYPES ==================

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

// ---- paste the rest of your implementation here ----
// (copy_str, key_equality, fnv1a_32_str, init_bucket,
//  insert_existing_bucket, insert_new_bucket, destroy_bucket,
//  resize, hashmap_create, hashmap_put, hashmap_get,
//  hashmap_destroy, hahsmap_remove)
// ----------------------------------------------------

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

bool key_equality(char* key1, char* key2) {
    while(*key1 && *key2) {
        if(*key1 != *key2) return false;

        key1++;
        key2++;
    }

    return !(*key1 || *key2);
}
// ================== UTILITY FUNCTIONS ==================

// ================== HASH FUNCTION ==================

uint32_t fnv1a_32_str(char* key)  {
    uint32_t hash = FNV_32_OFFSET;
    
    while(*key) {
        hash ^= (uint32_t) (unsigned char) (*key);
        hash *= FNV_32_PRIME;
        key++;
    }
    
    return hash;
}

// ================== HASH FUNCTION ==================

// ================== HELPER FUNCTIONS ==================

bucket* init_bucket(char* key, void* value) {
    bucket* b = malloc(sizeof(bucket));
    b->key = copy_str(key);
    b->value = value;
    b->next = NULL;

    return b;
}

void insert_existing_bucket(
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

void destroy_bucket(bucket* b) {
    b->next = NULL;
    free(b->key);
    free(b);
}

bucket** resize(hashmap* map) {
    int old_capacity = map->capacity;

    bucket** old_buckets = map->buckets;
    bucket** new_buckets = calloc(old_capacity * 2, sizeof(bucket*));

    for(int idx = 0; idx < old_capacity; idx++) {
        bucket* b = old_buckets[idx];

        bucket* next;

        while(b != NULL) {
            next = b->next; 
            b->next = NULL;
            insert_existing_bucket(new_buckets, b, old_capacity * 2);
            b = next;
        }
    }

    return new_buckets;
}
// ================== HELPER FUNCTIONS ==================

// ================== PUBLIC FUNCTIONS ==================
hashmap* hashmap_create() {
    hashmap* map = malloc(sizeof(hashmap));
    
    map->capacity = INITIAL_CAPACITY;
    map->size = 0;
    
    map->buckets = calloc(INITIAL_CAPACITY, sizeof(bucket*));
    
    return map;
}


void hashmap_put(hashmap* map, char* key, void* value) {
    float load_factor = (float) map->size / map->capacity;
    
    if (load_factor > 0.75) {
        bucket** old_buckets = map->buckets;
        bucket** new_buckets = resize(map);
        free(old_buckets);
        map->capacity *= 2;
        map->buckets = new_buckets;
    }
    
    bool new_key_was_added = insert_new_bucket(map->buckets, key, value, map->capacity);
    
    if(new_key_was_added) map->size++; 
}

void hashmap_destroy(hashmap* map) {
    for(int idx = 0; idx < map->capacity; idx++) {
        bucket* b = map->buckets[idx];
        
        bucket* temp;
        while(b != NULL) {
            temp = b->next;
            destroy_bucket(b);
            b = temp;
        }
    }
    
    free(map->buckets);
    free(map);
}

void* hashmap_get(hashmap* map, char* key) {
    uint32_t hash = fnv1a_32_str(key);
    size_t idx = hash % map->capacity;
    
    bucket* b = map->buckets[idx];
    
    while(b != NULL) {
        if (key_equality(b->key, key)) return b->value;
        b = b->next;
    }
    
    return NULL;
}

bool hashmap_remove(hashmap* map, char* key) {
    uint32_t hash = fnv1a_32_str(key);
    size_t idx = hash % map->capacity;
    
    bucket* b = map->buckets[idx];
    
    bucket* prev = NULL;
    while(b != NULL) {
        if(key_equality(b->key, key)) {
            if(prev == NULL) {
                map->buckets[idx] = b->next;
            }
            
            else {
                prev->next = b->next;
            }
            
            map->size--;
            destroy_bucket(b);
            
            return true;
        }
        
        prev = b;
        b = b->next;
    }
    
    return false;
}

// ================== PUBLIC FUNCTIONS ==================

// ================== TEST HELPERS ==================

static int tests_run    = 0;
static int tests_passed = 0;

#define CHECK(label, expr)                                          \
    do {                                                            \
        tests_run++;                                                \
        if (expr) {                                                 \
            printf("  [PASS] %s\n", label);                        \
            tests_passed++;                                         \
        } else {                                                    \
            printf("  [FAIL] %s  (line %d)\n", label, __LINE__);   \
        }                                                           \
    } while (0)

// ================== TEST CASES ==================

// --- hashmap_create ---
void test_create() {
    printf("\n=== hashmap_create ===\n");
    hashmap* m = hashmap_create();

    CHECK("map is not NULL",           m != NULL);
    CHECK("initial size is 0",         m->size == 0);
    CHECK("initial capacity is set",   m->capacity == INITIAL_CAPACITY);
    CHECK("buckets array allocated",   m->buckets != NULL);

    hashmap_destroy(m);
}

// --- hashmap_put ---
void test_put() {
    printf("\n=== hashmap_put ===\n");
    hashmap* m = hashmap_create();

    int v1 = 42, v2 = 99;
    hashmap_put(m, "alpha", &v1);
    CHECK("size is 1 after first insert",   m->size == 1);

    hashmap_put(m, "beta", &v2);
    CHECK("size is 2 after second insert",  m->size == 2);

    // Updating an existing key must NOT grow size
    int v3 = 7;
    hashmap_put(m, "alpha", &v3);
    CHECK("size unchanged after key update", m->size == 2);

    hashmap_destroy(m);
}

// --- hashmap_get ---
void test_get() {
    printf("\n=== hashmap_get ===\n");
    hashmap* m = hashmap_create();

    int val = 123;
    hashmap_put(m, "mykey", &val);

    void* result = hashmap_get(m, "mykey");
    CHECK("get returns correct pointer",     result == &val);
    CHECK("get returns correct value",       *(int*)result == 123);
    CHECK("get missing key returns NULL",    hashmap_get(m, "nope") == NULL);

    // Update value and re-get
    int new_val = 456;
    hashmap_put(m, "mykey", &new_val);
    result = hashmap_get(m, "mykey");
    CHECK("get reflects updated value",      *(int*)result == 456);

    hashmap_destroy(m);
}

// --- hahsmap_remove (note: typo is in your source) ---
void test_remove() {
    printf("\n=== hashmap_remove ===\n");
    hashmap* m = hashmap_create();

    int a = 1, b = 2, c = 3;
    hashmap_put(m, "x", &a);
    hashmap_put(m, "y", &b);
    hashmap_put(m, "z", &c);

    // Remove middle key
    bool removed = hashmap_remove(m, "y");
    CHECK("remove returns true for existing key",   removed == true);
    CHECK("size decremented after remove",          m->size == 2);
    CHECK("removed key is no longer gettable",      hashmap_get(m, "y") == NULL);
    CHECK("other keys still accessible",            hashmap_get(m, "x") == &a);

    // Remove non-existent key
    bool not_removed = hashmap_remove(m, "zzz");
    CHECK("remove returns false for missing key",   not_removed == false);
    CHECK("size unchanged after failed remove",     m->size == 2);

    // Remove remaining keys
    hashmap_remove(m, "x");
    hashmap_remove(m, "z");
    CHECK("size is 0 after all removes",            m->size == 0);

    hashmap_destroy(m);
}

// --- resize (triggered automatically via put) ---
void test_resize() {
    printf("\n=== resize (via hashmap_put) ===\n");
    hashmap* m = hashmap_create();

    int vals[20];
    char key[16];
    int cap_before = m->capacity;

    // Insert enough to force at least one resize
    for (int i = 0; i < 20; i++) {
        vals[i] = i;
        snprintf(key, sizeof(key), "key%d", i);
        hashmap_put(m, key, &vals[i]);
    }

    CHECK("capacity grew after many inserts",   m->capacity > cap_before);
    CHECK("size is 20 after 20 inserts",        m->size == 20);

    // Verify all values still retrievable after resize
    bool all_ok = true;
    for (int i = 0; i < 20; i++) {
        snprintf(key, sizeof(key), "key%d", i);
        void* r = hashmap_get(m, key);
        if (r == NULL || *(int*)r != i) { all_ok = false; break; }
    }
    CHECK("all values intact after resize",     all_ok);

    hashmap_destroy(m);
}

// --- collision handling ---
void test_collisions() {
    printf("\n=== collision handling ===\n");

    // We can't guarantee a collision with arbitrary keys, so we
    // just verify that a dense insertion + retrieval round-trip
    // is correct — if chaining is broken, something will come back NULL.
    hashmap* m = hashmap_create();
    int vals[50];
    char key[16];

    for (int i = 0; i < 50; i++) {
        vals[i] = i * 10;
        snprintf(key, sizeof(key), "k%d", i);
        hashmap_put(m, key, &vals[i]);
    }

    bool all_ok = true;
    for (int i = 0; i < 50; i++) {
        snprintf(key, sizeof(key), "k%d", i);
        void* r = hashmap_get(m, key);
        if (r == NULL || *(int*)r != i * 10) { all_ok = false; break; }
    }
    CHECK("50 keys retrieved correctly (stress + collision)", all_ok);

    hashmap_destroy(m);
}

// ================== MAIN ==================

int main() {
    test_create();
    test_put();
    test_get();
    test_remove();
    test_resize();
    test_collisions();

    printf("\n==============================\n");
    printf("Results: %d / %d tests passed\n", tests_passed, tests_run);
    printf("==============================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
