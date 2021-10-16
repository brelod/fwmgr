#pragma once

#include <stdbool.h>
#include <pthread.h>


typedef struct {
    int size;
    int head;
    int tail;
    void **nodes;
    pthread_mutex_t lock;
} queue_t;

queue_t* queue_create(int size);
int queue_put(queue_t *q, void *data);
void* queue_get(queue_t *q);
void queue_destroy(queue_t *q);
