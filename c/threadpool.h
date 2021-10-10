#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdbool.h>
#include "queue.h"


enum tp_state {TP_NONE, TP_RUNNING, TP_STOPPED};

struct tp_job {
    void (*function)(void *arg);
    void *arg;
};

struct tp_worker {
    int id;
    enum tp_state state; 
    pthread_t thid;
    pthread_cond_t ready;
    pthread_mutex_t mutex;
    struct tp *tp;
    struct tp_job *job;
};

struct tp_manager {
    int id;
    enum tp_state state; 
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

tp_t* tp_create(int workers, int jobs);
int tp_start(tp_t *tp);
int tp_exec(tp_t *tp, tp_job_t *job);
void tp_stop(tp_t *tp);
void tp_destroy(tp_t *tp);

#endif
