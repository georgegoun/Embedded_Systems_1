#include "../include/timer.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define QUEUESIZE 10
#define LOOP 100
#define p 2
#define c 2

void* producer(void* args);
void* consumer(void* args);

typedef struct {
    int buf[QUEUESIZE];
    long head, tail;
    int full, empty;
    pthread_mutex_t* mut;
    pthread_cond_t *notFull, *notEmpty;
} queue;

typedef struct workFunction {
    void* (*work)(void*);
    void* arg;
    struct timespec start;
    struct timespec stop;
} workStruct;

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

    workStruct sthread;

    for (i = 0; i < LOOP; i++) {
        pthread_mutex_lock(fifo->mut);
        while (fifo->full) {
            printf("producer: queue FULL.\n");
            pthread_cond_wait(fifo->notFull, fifo->mut);
        }
        sthread.start = timerStart(sthread.start);

        //TODO: Activate Function

        //TODO: Pass workFunc struct to fifo struct
        //fifo
        queueAdd(fifo, i);

        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notEmpty);
    }
    return (NULL);
}

void* consumer(void* q)
{
    queue* fifo;
    int i, d;

    // printf("ok con");

    fifo = (queue*)q;

    // original ex
    //  for (i = 0; i < LOOP; i++) {
    //      pthread_mutex_lock(fifo->mut);
    //      while (fifo->empty) {
    //          printf("consumer: queue EMPTY.\n");
    //          pthread_cond_wait(fifo->notEmpty, fifo->mut);
    //      }
    //      queueDel(fifo, &d);
    //      pthread_mutex_unlock(fifo->mut);
    //      pthread_cond_signal(fifo->notFull);
    //      printf("consumer: recieved %d.\n", d);
    //  }
    while (1) {
        pthread_mutex_lock(fifo->mut);
        while (fifo->empty) {
            printf("consumer: queue EMPTY.\n");
            pthread_cond_wait(fifo->notEmpty, fifo->mut);
        }
        queueDel(fifo, &d);
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notFull);
        printf("consumer: recieved %d.\n", d);
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
    printf("Consumer thread: executed!\n");
    return 0;
}