#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdint.h>

typedef struct {
    uint32_t index;
    float x;
    float y;
} city_t;

typedef struct {
    uint8_t depth;
    city_t  cities[30];
} Path_t;

typedef struct Node {
    Path_t data;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
} LockFreeQueue;

LockFreeQueue* createQueue();

void enqueue(LockFreeQueue* queue, Path_t data);

Path_t dequeue(LockFreeQueue* queue);

void destroyQueue(LockFreeQueue* queue);

#endif