#pragma once

#include <stdbool.h>

struct queue_node {
    struct queue_node *next;
    void *data;
};

struct queue_head {
    int size;
    struct queue_node *first;
    struct queue_node *last;
};

typedef struct queue_head queue_t;

queue_t* queue_create(size_t size);
void queue_destroy(queue_t *q);
bool queue_isfull(queue_t *q);
bool queue_isempty(queue_t *q);
int queue_put(queue_t *q, void *data);
void* queue_get(queue_t *q);
void* queue_peek(queue_t *q);
