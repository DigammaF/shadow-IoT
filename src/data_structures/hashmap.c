
#include <stdlib.h>
#include <stdio.h>

#include "hashmap.h"

#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }

#define CHECKKEY(hashmap, key) { if (key > hashmap->capacity) { fprintf(stderr, "%i is not in hashmap bounds [0-%i[\n", key, hashmap->capacity); exit(EXIT_FAILURE); } }

void initHashmap(hashmap_t* hashmap) {
    hashmap->capacity = 1;
    hashmap->elements = malloc(sizeof(void*));
	hashmap->elements[0] = NULL;
    CHECKM(hashmap->elements, "malloc hashmap->elements");
}

void dropHashmap(hashmap_t* hashmap) {
    free(hashmap->elements);
}

void* hashmapGet(hashmap_t* hashmap, unsigned key) {
    if (key >= hashmap->capacity) { return NULL; }
    return hashmap->elements[key];
}

void hashmapSet(hashmap_t* hashmap, unsigned key, void* value) {
    while (key >= hashmap->capacity) { increaseHashmapCapacity(hashmap); }
    hashmap->elements[key] = value;
}

unsigned hashmapLocateUnusedKey(hashmap_t* hashmap) {
	for (unsigned key = 0; key < hashmap->capacity; key++) {
		if (hashmap->elements[key] == NULL) {
			return key;
		}
	}

	unsigned key = hashmap->capacity;
	increaseHashmapCapacity(hashmap);
	return key;
}

unsigned hashmapValueCount(hashmap_t* hashmap) {
	unsigned count = 0;

	for (unsigned n = 0; n < hashmap->capacity; n++) {
		if (hashmap->elements[n] != NULL) { count++; }
	}

	return count;
}

void increaseHashmapCapacity(hashmap_t* hashmap) {
	unsigned previousCapacity = hashmap->capacity;
    unsigned newCapacity = hashmap->capacity*HASHMAP_GROWTH_RATE + HASHMAP_GROWTH_CONSTANT;
    if (newCapacity <= hashmap->capacity) { fprintf(stderr, "cannot increase hashmap capacity\n"); exit(EXIT_FAILURE); }
    hashmap->capacity = newCapacity;
    hashmap->elements = realloc(hashmap->elements, hashmap->capacity*sizeof(void*));
    CHECKM(hashmap->elements, "realloc(hashmap->elements)");

	for (unsigned n = previousCapacity; n < hashmap->capacity; n++) {
		hashmap->elements[n] = NULL;
	}
}

void dumpHashmap(hashmap_t* hashmap) {
	printf("--- Dumping Hashmap %i ---\n", *(unsigned*)hashmap);
	printf("capacity: %i\n", hashmap->capacity);
    for (unsigned n = 0; n < hashmap->capacity; n++) {
        if (hashmap->elements[n] == NULL) {
			printf("%i -> NULL\n", n);
			continue;
		}
        printf("%i -> %i\n", n, *(unsigned*)hashmap->elements[n]);
    }
}
