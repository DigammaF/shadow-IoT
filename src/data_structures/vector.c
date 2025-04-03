
#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

#define CHECKM(status, message) { if ((status) == NULL) { perror(message); exit(EXIT_FAILURE); } }

#define CHECKKEY(vector, key) { if (key > vector->length) { fprintf(stderr, "%i is not in vector bounds [0-%i[\n", key, vector->length); exit(EXIT_FAILURE); } }

void initVector(vector_t* vector) {
    vector->capacity = 1;
    vector->length = 0;
    vector->elements = malloc(sizeof(void*));
    CHECKM(vector->elements, "malloc vector->elements");
}

void dropVector(vector_t* vector) {
    free(vector->elements);
}

void* vectorGet(vector_t* vector, unsigned key) {
    CHECKKEY(vector, key);
    return vector->elements[key];
}

void vectorSet(vector_t* vector, unsigned key, void* value) {
    CHECKKEY(vector, key);
    vector->elements[key] = value;
}

void vectorAppend(vector_t* vector, void* value) {
    if (vector->length + 1 >= vector->capacity) { increaseCapacity(vector); }
    vector->elements[vector->length] = value;
    vector->length++;
}

void vectorInsert(vector_t* vector, unsigned key, void* value) {
    if (vector->length + 1 >= vector->capacity) { increaseCapacity(vector); }

    for (unsigned n = vector->length; n > key; n--) {
        vector->elements[n] = vector->elements[n - 1];
    }

    vector->elements[key] = value;
    vector->length++;
}

void* vectorPop(vector_t* vector, unsigned key) {
    void* value = vector->elements[key];

    for (unsigned n = key; n < vector->length; n++) {
        vector->elements[n] = vector->elements[n + 1];
    }

    vector->length--;
    return value;
}

void increaseCapacity(vector_t* vector) {
    unsigned newCapacity = vector->capacity*VECTOR_GROWTH_RATE + 3;
    if (newCapacity <= vector->capacity) { fprintf(stderr, "cannot increase vector capacity\n"); exit(EXIT_FAILURE); }
    vector->capacity = newCapacity;
    vector->elements = realloc(vector->elements, vector->capacity*sizeof(void*));
    CHECKM(vector->elements, "realloc(vector->elements)");
}

void dumpVector(vector_t* vector) {
    printf("\n- VECTOR DUMP -\n");

    for (unsigned n = 0; n < vector->length; n++) {
        //printf("%i ", (unsigned)vector->elements[n]);
    }

    printf("\n---\n");
}
