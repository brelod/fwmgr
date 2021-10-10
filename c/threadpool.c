#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "queue.h"
#include "logging.h"
#include "threadpool.h"


tp_worker_t* tp_find_worker(tp_t *tp)                   // TODO: this should be static
{
    for (int i=0; i<tp->size; ++i)
        if (tp->workers[i].state == TP_RUNNING && tp->workers[i].job == NULL)
            return &tp->workers[i];

    return NULL;
}

void* tp_manager_run(void *arg)
{
    tp_job_t *job = NULL;
    tp_worker_t *worker = NULL;
    tp_t *tp = (tp_t*) arg;
    tp_manager_t *self = tp->manager;

    self->state = TP_RUNNING;
    while (1) {
        // Wait for job
        while ((job = queue_get(tp->jobs)) == NULL) {
            if (self->state == TP_STOPPED)
                pthread_exit(0);

            if (pthread_mutex_lock(&self->job_mutex) != 0) {
                log_error("Failed lock mutex");
                exit(1);
            }
            if (pthread_cond_wait(&self->job_ready, &self->job_mutex) != 0) {
                log_error("Failed to wait for condition");
                exit(1);
            }
            if (pthread_mutex_unlock(&self->job_mutex) != 0) {
                log_error("Failed to unlock mutex");
                exit(1);
            }
        }

        // Wait for worker
        while ((worker = tp_find_worker(tp)) == NULL) {
            if (self->state == TP_STOPPED)
                pthread_exit(0);

            if (pthread_mutex_lock(&self->th_mutex) != 0) {
                log_error("Failed lock mutex");
                exit(1);
            }
            if (pthread_cond_wait(&self->th_ready, &self->th_mutex) != 0) {
                log_error("Failed to wait for condition");
                exit(1);
            }
            if (pthread_mutex_unlock(&self->th_mutex) != 0) {
                log_error("Failed to unlock mutex");
                exit(1);
            }
        }

        if (self->state != TP_RUNNING)
            pthread_exit(0);

        worker->job = job;
        pthread_cond_signal(&worker->ready);
    }
}

void* tp_worker_run(void *arg)
{
    tp_worker_t *self = (tp_worker_t*) arg;

    self->state = TP_RUNNING;
    while (1) {
        if (pthread_mutex_lock(&self->mutex) != 0) {
            log_error("Failed lock mutex");
            exit(1);
        }
        if (pthread_cond_wait(&self->ready, &self->mutex) != 0) {
            log_error("Failed to wait for condition");
            exit(1);
        }
        if (pthread_mutex_unlock(&self->mutex) != 0) {
            log_error("Failed to unlock mutex");
            exit(1);
        }

        if (self->state != TP_RUNNING)
            pthread_exit(0);

        if (self->job != NULL) {
            self->job->function(self->job->arg);
            free(self->job);                                    // TODO: this should be in the caller
            self->job = NULL;
            pthread_cond_signal(&self->tp->manager->th_ready);
        }

    }
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

        if (pthread_mutex_init(&worker->mutex, NULL) != 0) {
            perror("Failed to init worker mutex");
            return -1;
        }

        if (pthread_cond_init(&worker->ready, NULL) != 0) {
            perror("Failed to init worker cond");
            return -1;
        }

        worker->id = id;
        worker->tp = tp;
        worker->state = TP_NONE;
        if (pthread_create(&worker->thid, NULL, tp_worker_run, worker) != 0) {
            perror("Failed to create worker thread");
            return -1;
        }
    }

    tp->manager->state = TP_NONE;
    if (pthread_create(&tp->manager->thid, NULL, tp_manager_run, tp) != 0) {
        perror("Failed to create manager thread");
        return -1;
    }

    return 0;
}

int tp_job_run(tp_t *tp, tp_job_t *job)
{
    if (queue_put(tp->jobs, (void*)job) < 0)
        return -1;

    pthread_cond_signal(&tp->manager->job_ready);
    return 0;
}

void tp_stop(tp_t *tp)
{
    // Stop workers
    for (int i=0; i<tp->size; ++i) {
        tp->workers[i].state = TP_STOPPED;
        pthread_cond_signal(&tp->workers[i].ready);
    }

    // Stop manager
    tp->manager->state = TP_STOPPED;
    pthread_cond_signal(&tp->manager->job_ready);   // Stop waiting for job
    pthread_cond_signal(&tp->manager->th_ready);    // Stop waiting for worker
}

void tp_destroy(tp_t *tp)
{
    queue_destroy(tp->jobs);
    free(tp->workers);
    free(tp->manager);
    free(tp);
}


/*
int main()
{
    tp_t *tp = tp_create(4, 20);    // 4 threads 20 queue
    tp_start(tp);

    int numbers[15];
    tp_job_t job[15];

    for (int i=0; i<15; ++i)
        numbers[i] = i;

    for (int i=0; i<15; ++i) {
        job[i].function = job_function;
        job[i].arg = (void*)&numbers[i];

        tp_job_run(tp, &job[i]);
    }

    while (! queue_isempty(tp->jobs)) {
        printf("waiting...\n");
        usleep(500000);
    }
    printf("Jobs are done; Let's have a beer!\n");
    tp_stop(tp);
    tp_destroy(tp);
    printf("Threadpool destroyed\n");
    return 0;

    //pthread_exit(0);

    //for (int i=0; i<5; ++i) {
    //    printf("alma\n");

    //    job.function = job_function;
    //    job.arg = (void*)&i;

    //    usleep(200000);
    //}
}
*/
