
#ifndef VECTOR_H
#define VECTOR_H

#define VECTOR_GROWTH_RATE 1.1

typedef struct {
    void** elements;
    unsigned capacity;
    unsigned length;
} vector_t;

void initVector(vector_t* vector);
void dropVector(vector_t* vector);

void* vectorGet(vector_t* vector, unsigned key);
void vectorSet(vector_t* vector, unsigned key, void* value);

void vectorAppend(vector_t* vector, void* value);
void vectorInsert(vector_t* vector, unsigned key, void* value);
void* vectorPop(vector_t* vector, unsigned key);

void increaseCapacity(vector_t* vector);

void dumpVector(vector_t* vector);

#endif
