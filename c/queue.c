#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define LOG_ENABLE

#include "queue.h"
#include "logging.h"


static inline int _next(queue_t *q, int idx) { return ++idx % q->size; }
static inline bool _isempty(queue_t *q) { return q->nodes[q->head] == NULL; }
static inline bool _isfull(queue_t *q) { return  q->head == q->tail && ! _isempty(q); }


queue_t* queue_create(int size)
{
        queue_t *q = NULL;

        if (size < 1) {
                log_error("Invalid queue size: %d\n", size);
                return NULL;
        }

        // Setup the queue
        q = (queue_t*) calloc (1, sizeof(*q));
        if (q < 0) {
                log_error("Failed to calloc() memory for queue_head");
                return NULL;
        }

        // Setup the nodes
        q->nodes = (void*) calloc (size, sizeof(void*));
        if (q->nodes < 0) {
                log_error("Failed to calloc() memory for queue_nodes");
                return NULL;
        }

        q->size = size;
        q->head = 0;
        q->tail = 0;

        return q;
}

int queue_put(queue_t *q, void *data)
{
        pthread_mutex_lock(q->lock);
        if (_isfull(q))
                return -1;

        q->nodes[q->tail] = data;
        q->tail  = _next(q, q->tail);

        pthread_mutex_unlock(q->lock);
        return 0;
}

void* queue_get(queue_t *q)
{
        void *data;

        pthread_mutex_lock(q->lock);
        if (_isempty(q))
                return NULL;

        data = q->nodes[q->head];
        q->nodes[q->head] = NULL;
        q->head = _next(q, q->head);

        pthread_mutex_unlock(q->lock);
        return data;
}

void queue_destroy(queue_t *q)
{
        free(q->nodes);
        free(q);
}
