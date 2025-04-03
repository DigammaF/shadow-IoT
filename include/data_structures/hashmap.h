
#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>

#define HASHMAP_GROWTH_RATE 1.1
#define HASHMAP_GROWTH_CONSTANT 20

/**
 * 
 *  pas une vraie hashmap au sens où il n'y a pas de hash,
 *  mais se manipule comme tel
 * 
 */
typedef struct {
    void** elements;
    unsigned capacity;
} hashmap_t;

#define BLANK_HASHMAP { .elements = NULL, .capacity = 0 }

void initHashmap(hashmap_t* hashmap);
void dropHashmap(hashmap_t* hashmap);

void* hashmapGet(hashmap_t* hashmap, unsigned key);
void hashmapSet(hashmap_t* hashmap, unsigned key, void* value);

unsigned hashmapLocateUnusedKey(hashmap_t* hashmap);
unsigned hashmapValueCount(hashmap_t* hashmap);

void increaseHashmapCapacity(hashmap_t* hashmap);

void dumpHashmap(hashmap_t* hashmap);

#endif
