#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "queue.h"

LockFreeQueue* createQueue() {
    LockFreeQueue* queue = malloc(sizeof(LockFreeQueue));
    if (!queue) {
        perror("Failed to allocate memory for queue");
        exit(EXIT_FAILURE);
    }

    Node* dummy = malloc(sizeof(Node));
    if (!dummy) {
        perror("Failed to allocate memory for dummy node");
        exit(EXIT_FAILURE);
    }
    dummy->next = NULL;

    queue->head = dummy;
    queue->tail = dummy;
    return queue;
}

void enqueue(LockFreeQueue* queue, Path_t data) {
    Node* newNode = malloc(sizeof(Node));
    if(newNode == NULL) printf("failed malloc\n");
    if (!newNode) {
        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;

    Node* tail;
    Node* next;

    while (1) {
        tail = atomic_load(&queue->tail);
        next = atomic_load(&tail->next);
	asm("mfence");								// make sure that the variable is synchronized for all memories!

        if (tail == atomic_load(&queue->tail)) {
            if (next == NULL) {
                if (atomic_compare_exchange_weak(&tail->next, &next, newNode)) {
                    break;
                }
            } else {
                atomic_compare_exchange_weak(&queue->tail, &tail, next);
            }
        }
    }
    atomic_compare_exchange_weak(&queue->tail, &tail, newNode);
}

Path_t dequeue(LockFreeQueue* queue) {
    Node* head;
    Node* tail;
    Node* next;
    Path_t data;

    while (1) {
        head = atomic_load(&queue->head);
        tail = atomic_load(&queue->tail);
        next = atomic_load(&head->next);
	asm("mfence");								// make sure that the variable is synchronized for all memories!

        if (head == atomic_load(&queue->head)) {
            if (head == tail) {
                while (next == NULL){
                    head = atomic_load(&queue->head);
                    tail = atomic_load(&queue->tail);
                    next = atomic_load(&head->next);
                };
                atomic_compare_exchange_weak(&queue->tail, &tail, next);
            } else {
                data = next->data;
                if (atomic_compare_exchange_weak(&queue->head, &head, next)) {
                    return data;               
                    free(head);
                }
            }
        }
    }
}

void destroyQueue(LockFreeQueue* queue) {
    Node* current = queue->head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
    free(queue);
}