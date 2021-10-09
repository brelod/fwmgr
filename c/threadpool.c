#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define WORKERS 4

struct thread {
    int id;
    pthread_t thid;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    int operation;
    void *data;
};

void* manager(void *arg)
{
    char c;
    struct thread *workers = (struct thread*) arg;

    while (1) {
        c = getchar() & 0xf;
        getchar();
        pthread_cond_signal(&workers[c].cond);
    }
}

void* worker(void *arg)
{
    int i=0;
    struct thread *self = (struct thread*) arg;

    while (1) {
        if (pthread_mutex_lock(&self->mutex) != 0) {
            printf("Failed lock mutex\n");
            exit(1);
        }
        if (pthread_cond_wait(&self->cond, &self->mutex) != 0) {
            printf("Failed to wait for condition\n");
            exit(1);
        }
        if (pthread_mutex_unlock(&self->mutex) != 0) {
            printf("Failed to unlock mutex");
            exit(1);
        }
        printf("thread-id = %d\n", self->id);
    }

    return NULL;
}


int main()
{
    struct thread workers[WORKERS];

    memset(workers, 0, sizeof(workers));

    for (int i=0; i<WORKERS; ++i) {
        if (pthread_mutex_init(&workers[i].mutex, NULL) != 0) {
            perror("Failed to init mutex");
            exit(1);
        }

        if (pthread_cond_init(&workers[i].cond, NULL) != 0) {
            perror("Failed to init cond");
            exit(1);
        }

        workers[i].id = i;
        pthread_create(&workers[i].thid, NULL, worker, &workers[i]);
    }

    pthread_t th;
    pthread_create(&th, NULL, manager, &workers);

    pthread_exit(NULL);
}
