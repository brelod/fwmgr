#include <stdio.h>
#include <stdlib.h>
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


queue_t* queue_create(int size)
{
    queue_t *q = NULL;
    struct queue_node *node = NULL;

    q = (queue_t*) calloc (1, sizeof(queue_t));
    if (q < 0) {
        perror("Failed to calloc() memory for queue_head");
        exit(1);
    }

    node = (struct queue_node*) calloc (1, sizeof(struct queue_node));
    if (node < 0) {
        perror("Failed to calloc() memory for queue_node");
        exit(1);
    }

    q->first = node;
    q->last = node;

    for (int i=1; i<size; ++i) {
        node->next = (struct queue_node*) calloc (1, sizeof(struct queue_node));
        if (node < 0) {
            perror("Failed to calloc() memory for queue_node");
            exit(1);
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



/*
int main()
{
    int a = 1;
    char b = 'b';

    queue_t *queue = queue_create(3);
    queue_put(queue, (void*)&a);
    queue_put(queue, (void*)&b);

    int *aptr  = (int* ) queue_get(queue);
    char *bptr = (char*) queue_get(queue);
    printf("a=%d\n", *aptr);
    printf("b=%c\n", *bptr);

    queue_destroy(queue);
    queue = NULL;
}
*/
