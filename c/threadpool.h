#pragma once

#include <stdbool.h>
#include "queue.h"


enum tp_state {TP_NONE, TP_RUNNING, TP_STOPPED};

typedef struct tp_job {
        void (*function)(void *arg);
        void *arg;
} tp_job_t;

typedef struct tp_jobs {
        queue_t *pending;
        queue_t *finished;
} tp_jobs_t;

typedef struct tp_worker {
        pthread_t id;
        struct tp *tp;
        struct tp_job *job;
        enum tp_state state; 
        pthread_cond_t ready;
        pthread_mutex_t lock;
} tp_worker_t;

typedef struct tp_manager {
        pthread_t id;
        struct tp *tp;
        enum tp_state state; 
        pthread_cond_t job_ready;
        pthread_mutex_t job_lock;
        pthread_cond_t worker_ready;
        pthread_mutex_t worker_lock;
} tp_manager_t;

typedef struct tp {
        int size;
        struct tp_jobs jobs;
        struct tp_worker *workers;
        struct tp_manager *manager;
} tp_t;


tp_t* tp_create(int workers, int jobs);
void tp_destroy(tp_t *tp);

int tp_start(tp_t *tp);
void tp_stop(tp_t *tp);

int tp_put(tp_t *tp, tp_job_t *job);
tp_job_t* tp_get(tp_t *tp);
