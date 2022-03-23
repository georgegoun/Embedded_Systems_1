#include "../include/timer.h"

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define QUEUESIZE 10
#define LOOP 125
#define p 16
#define c 16

void* producer(void* args);
void* consumer(void* args);

typedef struct workFunction {
    void* (*work)(void*);
    void* arg;
    struct timespec start;
    struct timespec stop;
} workStruct;

typedef struct {
    int buf[QUEUESIZE];
    long head, tail;
    int full, empty;
    pthread_mutex_t* mut;
    pthread_cond_t *notFull, *notEmpty;
    workStruct nested_Struct;
} queue;

queue* queueInit(void);
void queueDelete(queue* q);
void queueAdd(queue* q, int in);
void queueDel(queue* q, int* out);
void* workFunc(void* args);

int main()
{
    queue* fifo;
    pthread_t* pro;
    pthread_t* con;

    pro = (pthread_t*)malloc(p * sizeof(*pro));
    con = (pthread_t*)malloc(c * sizeof(*con));

    fifo = queueInit();
    if (fifo == NULL) {
        fprintf(stderr, "main: Queue Init failed.\n");
        exit(1);
    }

    for (int i = 0; i < p; i++) {
        pthread_create(&pro[i], NULL, producer, fifo);
    }
    for (int j = 0; j < c; j++) {
        pthread_create(&con[j], NULL, consumer, fifo);
    }
    for (int i = 0; i < p; i++) {
        pthread_join(pro[i], NULL);
    }
    for (int j = 0; j < c; j++) {
        pthread_join(con[j], NULL);
    }
    queueDelete(fifo);

    return 0;
}

void* producer(void* q)
{
    queue* fifo;
    int i;

    fifo = (queue*)q;

    for (i = 0; i < LOOP; i++) {
        pthread_mutex_lock(fifo->mut);
        while (fifo->full) {
            // commented prints
            //printf("producer: queue FULL.\n");
            pthread_cond_wait(fifo->notFull, fifo->mut);
        }

        fifo->nested_Struct.start = timerStart(fifo->nested_Struct.start);

        queueAdd(fifo, i);

        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notEmpty);
    }
    return (NULL);
}

void* consumer(void* q)
{
    queue* fifo;
    int d;
    int counter = 0;

    fifo = (queue*)q;

    while (1) {
        counter++;
        pthread_mutex_lock(fifo->mut);
        while (fifo->empty) {
            // commented prints
            //printf("consumer: queue EMPTY.\n");
            pthread_cond_wait(fifo->notEmpty, fifo->mut);
        }
        queueDel(fifo, &d);
        fifo->nested_Struct.stop = timerStart(fifo->nested_Struct.stop);
        fifo->nested_Struct.work = workFunc(NULL);
        double time_dif = timeDif(fifo->nested_Struct.start, fifo->nested_Struct.stop);
        printf("%lf\n", time_dif);
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notFull);
        // commented prints
        //printf("consumer: recieved %d.\n", d);
    }
    return (NULL);
}

/*
  typedef struct {
  int buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
  } queue;
*/

queue* queueInit(void)
{
    queue* q;

    q = (queue*)malloc(sizeof(queue));
    if (q == NULL)
        return (NULL);

    q->empty = 1;
    q->full = 0;
    q->head = 0;
    q->tail = 0;
    q->mut = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(q->mut, NULL);
    q->notFull = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(q->notFull, NULL);
    q->notEmpty = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(q->notEmpty, NULL);

    return (q);
}

void queueDelete(queue* q)
{
    pthread_mutex_destroy(q->mut);
    free(q->mut);
    pthread_cond_destroy(q->notFull);
    free(q->notFull);
    pthread_cond_destroy(q->notEmpty);
    free(q->notEmpty);
    free(q);
}

void queueAdd(queue* q, int in)
{
    q->buf[q->tail] = in;
    q->tail++;
    if (q->tail == QUEUESIZE)
        q->tail = 0;
    if (q->tail == q->head)
        q->full = 1;
    q->empty = 0;

    return;
}

void queueDel(queue* q, int* out)
{
    *out = q->buf[q->head];

    q->head++;
    if (q->head == QUEUESIZE)
        q->head = 0;
    if (q->head == q->tail)
        q->empty = 1;
    q->full = 0;

    return;
}

void* workFunc(void* args)
{
    double res = 0.0;
    double j = 0.1;
    for (int i = 0; i < 10; i++) {
        res += sin(j);
        j += 0.05;
    }
    return 0;
}