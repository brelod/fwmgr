#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "queue.c"


#define WORKERS 4

struct tp_job {
    void (*function)(void *arg);
    void *arg;
};

struct tp_worker {
    int id;
    pthread_t thid;
    pthread_cond_t ready;
    pthread_mutex_t mutex;
    struct tp *tp;
    struct tp_job *job;
};

struct tp_manager {
    int id;
    pthread_t thid;
    pthread_cond_t th_ready;
    pthread_mutex_t th_mutex;
    pthread_cond_t job_ready;
    pthread_mutex_t job_mutex;
    struct tp *tp;
};

struct tp {
    int size;
    queue_t *jobs;
    struct tp_worker *workers;
    struct tp_manager *manager;
};

typedef struct tp tp_t;
typedef struct tp_job tp_job_t;
typedef struct tp_worker tp_worker_t;
typedef struct tp_manager tp_manager_t;

tp_worker_t* tp_find_worker(tp_t *tp)                   // TODO: this should be static
{
    for (int i=0; i<tp->size; ++i)
        if (tp->workers[i].job == NULL)
            return &tp->workers[i];

    return NULL;
}

void* tp_manager_run(void *arg)
{
    tp_job_t *job = NULL;
    tp_worker_t *worker = NULL;
    tp_t *tp = (tp_t*) arg;
    tp_manager_t *self = tp->manager;

    while (1) {
        // Wait for job
        while ((job = queue_get(tp->jobs)) == NULL) {
            if (pthread_mutex_lock(&self->job_mutex) != 0) {
                printf("Failed lock mutex\n");
                exit(1);
            }
            if (pthread_cond_wait(&self->job_ready, &self->job_mutex) != 0) {
                printf("Failed to wait for condition\n");
                exit(1);
            }
            if (pthread_mutex_unlock(&self->job_mutex) != 0) {
                printf("Failed to unlock mutex");
                exit(1);
            }
        }
        printf("MGR - Found job\n");

        // Wait for worker
        while ((worker = tp_find_worker(tp)) == NULL) {
            if (pthread_mutex_lock(&self->th_mutex) != 0) {
                printf("Failed lock mutex\n");
                exit(1);
            }
            if (pthread_cond_wait(&self->th_ready, &self->th_mutex) != 0) {
                printf("Failed to wait for condition\n");
                exit(1);
            }
            if (pthread_mutex_unlock(&self->th_mutex) != 0) {
                printf("Failed to unlock mutex");
                exit(1);
            }
        }
        printf("MGR - Found worker\n");

        worker->job = job;
        pthread_cond_signal(&worker->ready);
    }
}

void* tp_worker_run(void *arg)
{
    tp_worker_t *self = (tp_worker_t*) arg;

    while (1) {
        printf("Waiting for new job\n");

        if (pthread_mutex_lock(&self->mutex) != 0) {
            printf("Failed lock mutex\n");
            exit(1);
        }
        if (pthread_cond_wait(&self->ready, &self->mutex) != 0) {
            printf("Failed to wait for condition\n");
            exit(1);
        }
        if (pthread_mutex_unlock(&self->mutex) != 0) {
            printf("Failed to unlock mutex");
            exit(1);
        }
        printf("New jobs is started on Thread-%d\n", self->id);
        if (self->job == NULL) {
            perror("Thread was released without job");
            exit(1);
        }
        else 
            self->job->function(self->job->arg);
    }

    return NULL;
}

tp_t* tp_create(int workers, int jobs)
{
    tp_t *tp = (tp_t*) calloc (1, sizeof(tp_t));
    if (tp < 0)
        return NULL;

    tp->manager = (tp_manager_t*) calloc (1, sizeof(tp_manager_t));
    if (tp->manager < 0) {
        free(tp);
        return NULL;
    }

    tp->size = workers;
    tp->workers = (tp_worker_t*) calloc (workers, sizeof(tp_worker_t));
    if (tp->workers < 0) {
        free(tp->manager);
        free(tp);
        return NULL;
    }

    tp->jobs = queue_create(jobs);
    if (tp->jobs == NULL) {
        free(tp->workers);
        free(tp->manager);
        free(tp);
        return NULL;
    }

    return tp;
}

int tp_start(tp_t *tp)
{
    int id;
    tp_worker_t *worker; 

    // Init thread semaphores
    if (pthread_mutex_init(&tp->manager->th_mutex, NULL) != 0) {
        perror("Failed to init manager th_mutex");
        return -1;
    }
    if (pthread_cond_init(&tp->manager->th_ready, NULL) != 0) {
        perror("Failed to init manager th_ready");
        return -1;
    }

    // Init job semaphores
    if (pthread_mutex_init(&tp->manager->job_mutex, NULL) != 0) {
        perror("Failed to init manager job_mutex");
        return -1;
    }
    if (pthread_cond_init(&tp->manager->job_ready, NULL) != 0) {
        perror("Failed to init manager job_ready");
        return -1;
    }

    
    // Init and start worker threads
    for (id=0; id<tp->size; ++id) {
        worker = &tp->workers[id];
        worker->id = id;
        worker->tp = tp;

        if (pthread_mutex_init(&worker->mutex, NULL) != 0) {
            perror("Failed to init worker mutex");
            return -1;
        }

        if (pthread_cond_init(&worker->ready, NULL) != 0) {
            perror("Failed to init worker cond");
            return -1;
        }

        pthread_create(&worker->thid, NULL, tp_worker_run, &worker);
    }

    pthread_create(&tp->manager->thid, NULL, tp_manager_run, tp);
    return 0;
}

void tp_job_run(tp_t *tp, tp_job_t *job)
{
}

void tp_stop(tp_t *tp)
{
}

void tp_destroy(tp_t *tp)
{
}

void job_function(void *arg)
{
    printf("Job %d\n", *((int*)arg));
}

int main()
{
    tp_t *tp = tp_create(4, 4);
    tp_start(tp);

    int i = 1;
    tp_job_t job = {job_function, (void*)&i};

    queue_put(tp->jobs, (void*)&job);
    pthread_cond_signal(&tp->manager->job_ready);

    sleep(1);
    //for (int i=0; i<5; ++i) {
    //    printf("alma\n");

    //    job.function = job_function;
    //    job.arg = (void*)&i;

    //    usleep(200000);
    //}
}
