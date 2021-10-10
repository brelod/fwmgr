#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "queue.h"
#include "logging.h"


queue_t* queue_create(size_t size)
{
    queue_t *q = NULL;
    struct queue_node *node = NULL;

    if (size < 1) {
        log_error("Invalid queue size: %d\n", size);
        return NULL;
    }

    q = (queue_t*) calloc (1, sizeof(queue_t));
    if (q < 0) {
        log_error("Failed to calloc() memory for queue_head");
        return NULL;
    }
    q->size = size;

    node = (struct queue_node*) calloc (1, sizeof(struct queue_node));
    if (node < 0) {
        log_error("Failed to calloc() memory for queue_node");
        return NULL;
    }

    q->first = node;
    q->last = node;

    for (int i=1; i<size; ++i) {
        node->next = (struct queue_node*) calloc (1, sizeof(struct queue_node));
        if (node < 0) {
            log_error("Failed to calloc() memory for queue_node");
            return NULL;
        }
        node = node->next;
    }
    node->next = q->first;

    return q;
}

void queue_destroy(queue_t *q)
{
    struct queue_node *last = q->first;
    struct queue_node *node = last->next;

    if (q->size == 1) {
        free(q->first);
        free(q);
        return;
    }

    do {
        free(last);
        last = node;
        node = last->next;
    } while (node != q->first);

    free(q);
}

bool queue_isfull(queue_t *q)
{
    return (q->last == q->first && q->first->data != NULL) ? true : false;
}

bool queue_isempty(queue_t *q)
{
    return (q->last == q->first && q->first->data == NULL) ? true : false;
}

int queue_put(queue_t *q, void *data)
{
    if (queue_isfull(q))
        return -1;
    
    q->last->data = data;
    q->last = q->last->next;
    return 0;
}

void* queue_get(queue_t *q)
{
    void *data;

    if (queue_isempty(q))
        return NULL;

    data = q->first->data;
    q->first->data = NULL;
    q->first = q->first->next;

    return data;
}

void* queue_peek(queue_t *q)
{
    return q->first->data;
}
