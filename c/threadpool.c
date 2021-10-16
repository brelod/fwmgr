#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "queue.h"
#include "logging.h"
#include "threadpool.h"


static tp_worker_t* find_worker(tp_t *tp);
static void* start_manager(void *arg);
static void* start_worker(void *arg);


static tp_worker_t* find_worker(tp_t *tp)
{
    for (int i=0; i<tp->size; ++i)
        if (tp->workers[i].state == TP_RUNNING && tp->workers[i].job == NULL)
            return &tp->workers[i];

    return NULL;
}

static void wait_for_condition(pthread_mutex_t *lock, pthread_cond_t *ready)
{
        if (pthread_mutex_lock(lock) != 0) {
                log_error("Failed lock mutex");
                log_trace();
                pthread_exit(0);
        }
        if (pthread_cond_wait(ready, lock) != 0) {
                log_error("Failed to wait for condition");
                log_trace();
                pthread_exit(0);
        }
        if (pthread_mutex_unlock(lock) != 0) {
                log_error("Failed to unlock mutex");
                log_trace();
                pthread_exit(0);
        }
}

static void* start_manager(void *arg)
{
        tp_job_t *job = NULL;
        tp_worker_t *worker = NULL;
        tp_t *tp = (tp_t*) arg;
        tp_manager_t *self = tp->manager;

        log_debug("Threadpool manager was started");

        self->state = TP_RUNNING;
        while (1) {
                // Wait for job
                while ((job = queue_get(tp->jobs)) == NULL) {
                        if (self->state == TP_STOPPED)
                                goto stop_manager;
                        wait_for_condition(&self->job_mutex, &self->job_ready);
                }

                // Wait for worker
                while ((worker = find_worker(tp)) == NULL) {
                        if (self->state == TP_STOPPED)
                                goto stop_manager;
                        wait_for_condition(&self->th_mutex, &self->th_ready);
                }

                if (self->state == TP_STOPPED)
                        goto stop_manager;

                worker->job = job;
                pthread_cond_signal(&worker->ready);
        }

stop_manager:
        log_debug("Threadpool manager was stopped");
        pthread_exit(0);

}

static void* start_worker(void *arg)
{
        tp_worker_t *self = (tp_worker_t*) arg;

        log_debug("Worker thread was started");

        self->state = TP_RUNNING;
        while (1) {
                wait_for_condition(&self->mutex, &self->ready);

                if (self->state != TP_RUNNING) {
                        log_debug("Worker thread was stopped");
                        pthread_exit(0);
                }

                if (self->job != NULL) {
                    self->job->function(self->job->arg);
                    self->job = NULL;
                    pthread_cond_signal(&self->tp->manager->th_ready);
                }
        }
}

tp_t* tp_create(int workers, int jobs)
{
        tp_t *tp = NULL;

        log_debug("Creating threadpool with %d workers and job queue "
                        "with size %d", workers, jobs);

        tp = (tp_t*) calloc (1, sizeof(*tp));
        if (tp < 0) {
                log_error("Failed to calloc() memory for threadpool");
                return NULL;
        }

        tp->manager = (tp_manager_t*) calloc (1, sizeof(tp_manager_t));
        if (tp->manager < 0) {
                log_error("Failed to calloc() memory for threadpool manager");
                free(tp);
                return NULL;
        }

        tp->size = workers;
        tp->workers = (tp_worker_t*) calloc (workers, sizeof(tp_worker_t));
        if (tp->workers < 0) {
                log_error("Failed to calloc() memory for threadpool workers");
                free(tp->manager);
                free(tp);
                return NULL;
        }

        tp->jobs = queue_create(jobs);
        if (tp->jobs == NULL) {
                log_error("Failed to create job queue for threadpool");
                free(tp->workers);
                free(tp->manager);
                free(tp);
                return NULL;
        }

        log_debug("Threadpool has been created");
        return tp;
}

int tp_start(tp_t *tp)
{
    int id;
    tp_worker_t *worker; 
    log_debug("Starting threadpool");

    // Init thread conditional / mutex
    if (pthread_mutex_init(&tp->manager->th_mutex, NULL) != 0) {
        log_error("Failed to init manager th_mutex");
        return -1;
    }
    if (pthread_cond_init(&tp->manager->th_ready, NULL) != 0) {
        log_error("Failed to init manager th_ready");
        return -1;
    }

    // Init job conditional / mutex
    if (pthread_mutex_init(&tp->manager->job_mutex, NULL) != 0) {
        log_error("Failed to init manager job_mutex");
        return -1;
    }
    if (pthread_cond_init(&tp->manager->job_ready, NULL) != 0) {
        log_error("Failed to init manager job_ready");
        return -1;
    }
    
    // Init and start worker threads
    for (id=0; id<tp->size; ++id) {
        worker = &tp->workers[id];

        if (pthread_mutex_init(&worker->mutex, NULL) != 0) {
            log_error("Failed to init worker mutex");
            return -1;
        }

        if (pthread_cond_init(&worker->ready, NULL) != 0) {
            log_error("Failed to init worker cond");
            return -1;
        }

        worker->id = id;
        worker->tp = tp;
        worker->state = TP_NONE;
        if (pthread_create(&worker->thid, NULL, start_worker, worker) != 0) {
            log_error("Failed to create worker thread");
            return -1;
        }
    }

    tp->manager->state = TP_NONE;
    if (pthread_create(&tp->manager->thid, NULL, start_manager, tp) != 0) {
        log_error("Failed to create manager thread");
        return -1;
    }

    log_debug("Threadpool has started");
    return 0;
}

int tp_exec(tp_t *tp, tp_job_t *job)
{
    log_debug("Starting job");
    if (queue_put(tp->jobs, (void*)job) < 0)
        return -1;

    pthread_cond_signal(&tp->manager->job_ready);
    log_debug("Job has started");
    return 0;
}

void tp_stop(tp_t *tp)
{
    log_debug("Stopping threadpool");
    // Stop workers
    for (int i=0; i<tp->size; ++i) {
        tp->workers[i].state = TP_STOPPED;
        pthread_cond_signal(&tp->workers[i].ready); // Stop waiting for job
    }

    // Stop manager
    tp->manager->state = TP_STOPPED;
    pthread_cond_signal(&tp->manager->job_ready);   // Stop waiting for job
    pthread_cond_signal(&tp->manager->th_ready);    // Stop waiting for worker
    log_debug("Threadpool has stopped");
}

void tp_destroy(tp_t *tp)
{
    log_debug("Destroying threadpool");
    queue_destroy(tp->jobs);
    free(tp->workers);
    free(tp->manager);
    free(tp);
    log_debug("Threadpool has been destroyed");
}
